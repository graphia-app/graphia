#include "auth.h"

#include "shared/utils/preferences.h"
#include "shared/utils/string.h"

#include "../crashhandler.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QVariant>
#include <QLocale>
#include <QMessageBox>
#include <QSysInfo>

#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>

#include <vector>
#include <string>

#ifdef _DEBUG
//#define DEBUG_AUTH

#ifndef DEBUG_AUTH
#define DISABLE_AUTH
#endif
#endif

static std::string jsonObjectAsString(std::initializer_list<QPair<QString, QJsonValue>> args)
{
    return QJsonDocument(QJsonObject(args)).toJson(QJsonDocument::Compact).toStdString();
}

static std::string aesKeyAsJsonString(const u::AesKey& key)
{
    auto keyBaseHex = QByteArray(reinterpret_cast<const char*>(key._aes), // NOLINT
                                 sizeof(key._aes)).toHex();
    auto ivBaseHex = QByteArray(reinterpret_cast<const char*>(key._iv), // NOLINT
                                sizeof(key._iv)).toHex();

    auto keyJsonString = jsonObjectAsString(
    {
        {"aes", QString(keyBaseHex)},
        {"iv",  QString(ivBaseHex)}
    });

    return keyJsonString;
}

static QJsonObject decodeAuthResponse(const u::AesKey& aesKey, const std::string& authResponseJsonString)
{
    QJsonParseError jsonError{};
    auto jsonDocument = QJsonDocument::fromJson(authResponseJsonString.data(), &jsonError);

    if(jsonError.error != QJsonParseError::ParseError::NoError)
        return {};

    auto jsonObject = jsonDocument.object();
    auto aesKeySignatureHex = jsonObject[QStringLiteral("signature")].toString().toStdString();
    auto aesKeySignature = u::hexToString(aesKeySignatureHex);

    bool sessionVerified = u::rsaVerifySignature(aesKeyAsJsonString(aesKey),
        aesKeySignature, ":/keys/public_auth_key.der");
    if(!sessionVerified)
        return {};

    auto encryptedPayload = u::hexToBytes(jsonObject[QStringLiteral("payload")].toString().toStdString());

    auto payload = aesDecryptBytes(encryptedPayload, aesKey);

    jsonDocument = QJsonDocument::fromJson(payload.data(), &jsonError);
    if(jsonError.error != QJsonParseError::ParseError::NoError)
        return {};

    jsonObject = jsonDocument.object();

    return jsonObject;
}

static std::string authRequest(const u::AesKey& aesKey, const QString& email, const QString& encryptedPassword)
{
    auto payloadJsonString = jsonObjectAsString(
    {
        {"email",    email},
        {"password", encryptedPassword},
        {"locale",   QLocale::system().name()},
        {"product",  PRODUCT_NAME},
        {"version",  VERSION},
        {"os",       QString("%1 %2 %3 %4").arg(
            QSysInfo::kernelType(), QSysInfo::kernelVersion(),
            QSysInfo::productType(), QSysInfo::productVersion())}
    });

    auto encryptedAesKey = u::rsaEncryptString(aesKeyAsJsonString(aesKey), ":/keys/public_auth_key.der");
    auto encryptedPayload = aesEncryptString(payloadJsonString, aesKey);

    auto authReqJsonString = jsonObjectAsString(
    {
        {"key",     QString::fromStdString(encryptedAesKey)},
        {"payload", QString::fromStdString(encryptedPayload)}
    });

    return authReqJsonString;
}

void Auth::parseAuthToken()
{
    _issueTime = 0;
    _expiryTime = 0;
    _allowedPluginRegexps.clear();

    auto authToken = u::pref("auth/authToken").toString();

    if(authToken.isEmpty())
        return;

    auto encrypted = u::hexToString(authToken.toStdString());

    std::string aesKeyAndEncryptedAuthToken;
    if(!u::rsaVerifySignature(encrypted, ":/keys/public_auth_key.der", &aesKeyAndEncryptedAuthToken))
    {
        // If we get here, then someone is trying to manipulate the auth token
        return;
    }

    u::AesKey aesKey(aesKeyAndEncryptedAuthToken.c_str());

    auto encryptedAuthToken = aesKeyAndEncryptedAuthToken.substr(sizeof(aesKey._aes) + sizeof(aesKey._iv));
    auto decryptedAuthToken = aesDecryptString(encryptedAuthToken, aesKey);

    auto autoTokenJson = QJsonDocument::fromJson(decryptedAuthToken.data()).object();

    if(autoTokenJson.contains(QStringLiteral("issueTime")))
        _issueTime = autoTokenJson[QStringLiteral("issueTime")].toInt();

    if(autoTokenJson.contains(QStringLiteral("expiryTime")))
        _expiryTime = autoTokenJson[QStringLiteral("expiryTime")].toInt();

    if(autoTokenJson.contains(QStringLiteral("allowedPlugins")))
    {
        _allowedPluginRegexps.clear();
        auto value = autoTokenJson[QStringLiteral("allowedPlugins")].toArray();
        std::transform(value.begin(), value.end(), std::back_inserter(_allowedPluginRegexps),
        [](const auto& v)
        {
            return QRegularExpression(v.toString());
        });
    }
}

bool Auth::expired()
{
    parseAuthToken();

    auto now = static_cast<uint>(QDateTime::currentSecsSinceEpoch());
    auto approximatelyNow = now + 600; // Allow the system clock to be out by a little bit
    bool authorised = (approximatelyNow >= _issueTime) && (now < _expiryTime);

    if(_authorised != authorised)
    {
        _authorised = authorised;
        emit stateChanged();
    }

    // If we haven't succeeded by now, the token has expired
    return !_authorised;
}

Auth::Auth()
{
    connect(&_networkManager, &QNetworkAccessManager::finished,
            this, &Auth::onReplyReceived);

    QObject::connect(&_timer, &QTimer::timeout, this, &Auth::onTimeout);
}

void Auth::sendRequestUsingEncryptedPassword(const QString& email, const QString& encryptedPassword)
{
    if(_timer.isActive())
        return;

    _timer.setSingleShot(true);
    _timer.start(10000);
    emit busyChanged();

    _aesKey = u::generateAesKey();
    auto authReqJsonString = authRequest(_aesKey, email, encryptedPassword);

    // Work around for QTBUG-31652 (PRNG takes time to initialise) to
    // effectively call QNetworkAccessManager::post asynchronously
    QTimer::singleShot(0, [this, authReqJsonString]
    {
        QNetworkRequest request;
        request.setUrl(QUrl(QStringLiteral("https://auth.kajeka.com/")));

        auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(R"(form-data; name="request")"));
        part.setBody(authReqJsonString.data());
        multiPart->append(part);

#ifdef DISABLE_AUTH
        onReplyReceived();
#else
        _sslErrors.clear();
        _reply = _networkManager.post(request, multiPart);
        connect(_reply, &QNetworkReply::sslErrors, [this](const QList<QSslError>& errors)
        {
            _sslErrors = errors;
        });
        multiPart->setParent(_reply);
#endif
    });
}

void Auth::sendRequest(const QString& email, const QString& password)
{
    _encryptedPassword = QString::fromStdString(
        u::rsaEncryptString(password.toStdString(), ":/keys/public_auth_key.der"));

    sendRequestUsingEncryptedPassword(email, _encryptedPassword);
}

bool Auth::sendRequestUsingCachedCredentials()
{
    if(u::pref("auth/rememberMe").toBool())
    {
        auto email = u::pref("auth/emailAddress").toString();
        _encryptedPassword = u::pref("auth/password").toString();

        sendRequestUsingEncryptedPassword(email, _encryptedPassword);

        return true;
    }

    return false;
}

void Auth::reset()
{
    _authorised = false;
    _message.clear();
    _issueTime = 0;
    _expiryTime = 0;
    _allowedPluginRegexps.clear();

    emit stateChanged();
    emit messageChanged();

    if(_timer.isActive())
    {
        _timer.stop();
        emit busyChanged();

        if(_reply != nullptr)
        {
            _reply->abort();
            _reply->deleteLater();
            _reply = nullptr;
        }
    }

    u::setPref("auth/rememberMe", false);
}

bool Auth::pluginAllowed(const QString& pluginName) const
{
    if(_allowedPluginRegexps.empty())
        return true;

    return std::any_of(_allowedPluginRegexps.begin(), _allowedPluginRegexps.end(),
    [pluginName] (const auto& regex)
    {
        return regex.match(pluginName).hasMatch();
    });
}

void Auth::onReplyReceived()
{
    if(_timer.isActive())
    {
        _timer.stop();

#ifdef DISABLE_AUTH
        if(_reply == nullptr)
        {
            _authorised = true;
            emit stateChanged();
        }
        else
#endif

        if(_reply->error() == QNetworkReply::NetworkError::NoError)
        {
            std::string authResponse = _reply->readAll().toStdString();
            auto decodedRespose = decodeAuthResponse(_aesKey, authResponse);

            bool authorised = decodedRespose.contains(QStringLiteral("authenticated")) &&
                decodedRespose[QStringLiteral("authenticated")].toBool() &&
                decodedRespose.contains(QStringLiteral("authToken"));

            if(_authorised != authorised)
            {
                _authorised = authorised;

                u::setPref("auth/password", u::pref("auth/rememberMe").toBool() ?
                    _encryptedPassword : QLatin1String(""));

                if(_authorised)
                {
                    u::setPref("auth/authToken", decodedRespose[QStringLiteral("authToken")].toString());
                    parseAuthToken();

                    auto issueTime = QDateTime::fromSecsSinceEpoch(_issueTime);
                    auto now = QDateTime::currentDateTimeUtc();

                    if(std::abs(issueTime.secsTo(now)) > 600)
                    {
                        QMessageBox::warning(nullptr, tr("Clock Not Set"),
                            tr("Please ensure your system clock is accurately set and that the correct "
                               "timezone has been selected.\n\nFailure to set the system clock correctly "
                               "may prevent working offline."));
                    }
                }
                else
                    u::setPref("auth/authToken", "");

                emit stateChanged();
            }

            auto message = decodedRespose.contains(QStringLiteral("message")) ?
                decodedRespose[QStringLiteral("message")].toString() : QLatin1String("");

            if(_message != message)
            {
                _message = message;
                emit messageChanged();
            }
        }
        else if(expired())
        {
            auto message = tr("<b>NETWORK ERROR:</b> %1 (%2)")
                .arg(_reply->errorString())
                .arg(static_cast<int>(_reply->error()));

            if(!_sslErrors.isEmpty())
            {
                QString sslCodes;
                for(const auto& sslError: _sslErrors)
                {
                    if(!sslCodes.isEmpty())
                        sslCodes += tr(", ");
                    sslCodes += QString::number(static_cast<int>(sslError.error()));
                }

                message += tr(" [%s]").arg(sslCodes);
            }

            if(_message != message)
            {
                _message = message;
                emit messageChanged();
            }
        }

        if(_reply != nullptr && _reply->errorString().startsWith("TLS"))
            S(CrashHandler)->submitMinidump(_reply->errorString());

        emit busyChanged();
    }

    _encryptedPassword.clear();

    if(_reply != nullptr)
    {
        _reply->deleteLater();
        _reply = nullptr;
    }
}

void Auth::onTimeout()
{
    // Ignore timeouts if our token hasn't yet expired
    if(expired())
    {
        _message = tr("Timed out while waiting for a response from the authorisation "
                      "server. Please check your internet connection and try again.");
        emit messageChanged();
    }

    emit busyChanged();

    _reply->abort();
}
