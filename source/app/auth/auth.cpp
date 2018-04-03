#include "auth.h"

#include "thirdparty/cryptopp/cryptopp_disable_warnings.h"
#include "thirdparty/cryptopp/rsa.h"
#include "thirdparty/cryptopp/osrng.h"
#include "thirdparty/cryptopp/modes.h"
#include "thirdparty/cryptopp/cryptopp_enable_warnings.h"

#include "shared/utils/preferences.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QVariant>
#include <QLocale>
#include <QMessageBox>

#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>

#include <vector>
#include <sstream>
#include <string>
#include <iomanip>

#ifdef _DEBUG
//#define DEBUG_AUTH

#ifndef DEBUG_AUTH
#define DISABLE_AUTH
#endif
#endif

namespace C = CryptoPP;

static C::RSA::PublicKey loadPublicKey()
{
    QFile file(QStringLiteral(":/keys/public_auth_key.der"));
    if(!file.open(QIODevice::ReadOnly))
        return {};

    auto byteArray = file.readAll();
    file.close();

    C::ArraySource arraySource(reinterpret_cast<const byte*>(byteArray.constData()), // NOLINT
                               byteArray.size(), true);

    C::RSA::PublicKey publicKey;
    publicKey.Load(arraySource);

    return publicKey;
}

static Auth::AesKey generateKey()
{
    Auth::AesKey key;

    C::AutoSeededRandomPool rng;

    rng.GenerateBlock(static_cast<byte*>(key._aes), sizeof(key._aes));
    rng.GenerateBlock(static_cast<byte*>(key._iv), sizeof(key._iv));

    return key;
}

static std::string jsonObjectAsString(std::initializer_list<QPair<QString, QJsonValue>> args)
{
    return QJsonDocument(QJsonObject(args)).toJson(QJsonDocument::Compact).toStdString();
}

template<typename T>
static std::string bytesToHex(const T& bytes)
{
    std::ostringstream ss;

    ss << std::hex << std::setfill('0');
    for(int b : bytes)
    {
        if(b < 0)
            b += 0x100;

        ss << std::setw(2) << b;
    }

    return ss.str();
}

static bool isHex(const std::string& string)
{
    return string.size() % 2 == 0 &&
        QRegularExpression(QStringLiteral("^[a-fA-F0-9]+$")).match(QString::fromStdString(string)).hasMatch();
}

static std::vector<byte> hexToBytes(const std::string& string)
{
    std::vector<byte> bytes;
    bytes.reserve(string.length() / 2);

    if(isHex(string))
    {
        for(size_t i = 0; i < string.length(); i += 2)
        {
            auto byteString = string.substr(i, 2);
            auto b = static_cast<byte>(std::strtol(byteString.data(), nullptr, 16));
            bytes.push_back(b);
        }
    }

    return bytes;
}

static std::string hexToString(const std::string& string)
{
    std::string output;

    if(isHex(string))
    {
        for(size_t i = 0; i < string.length(); i += 2)
        {
            auto byteString = string.substr(i, 2);
            auto b = static_cast<char>(std::strtol(byteString.data(), nullptr, 16));
            output.push_back(b);
        }
    }

    return output;
}

static std::string rsaEncryptString(const std::string& string, const C::RSA::PublicKey& publicKey)
{
    C::RSAES_OAEP_SHA_Encryptor rsaEncryptor(publicKey);

    C::AutoSeededRandomPool rng;
    std::vector<byte> cipher(rsaEncryptor.FixedCiphertextLength());

    C::StringSource ss(string, true,
        new C::PK_EncryptorFilter(rng, rsaEncryptor,
            new C::ArraySink(cipher.data(), cipher.size())
       ) // PK_EncryptorFilter
    ); // StringSource
    Q_UNUSED(ss);

    return bytesToHex(cipher);
}

static std::string aesKeyAsJsonString(const Auth::AesKey& key)
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

static std::string rsaEncryptAesKey(const Auth::AesKey& key, const C::RSA::PublicKey& publicKey)
{
    return rsaEncryptString(aesKeyAsJsonString(key), publicKey);
}

static bool rsaVerifySignature(const std::string& string, const std::string& signature,
                               const C::RSA::PublicKey& publicKey)
{
    C::RSASSA_PKCS1v15_SHA_Verifier rsaVerifier(publicKey);
    std::string stringPlusSignature = (string + signature);

    try
    {
        C::StringSource ss(stringPlusSignature, true,
            new C::SignatureVerificationFilter(
                rsaVerifier, nullptr,
                C::SignatureVerificationFilter::THROW_EXCEPTION
           ) // SignatureVerificationFilter
        ); // StringSource
        Q_UNUSED(ss);
    }
    catch(C::SignatureVerificationFilter::SignatureVerificationFailed&)
    {
        return false;
    }

    return true;
}

static bool rsaVerifyAesKey(const Auth::AesKey& key, const std::string& signature, const C::RSA::PublicKey& publicKey)
{
    return rsaVerifySignature(aesKeyAsJsonString(key), signature, publicKey);
}

static std::string aesDecryptBytes(const std::vector<byte>& bytes, const Auth::AesKey& aesKey)
{
    std::vector<byte> outBytes(bytes.size());

    C::CFB_Mode<C::AES>::Decryption decryption(static_cast<const byte*>(aesKey._aes),
        sizeof(aesKey._aes), static_cast<const byte*>(aesKey._iv));
    decryption.ProcessData(outBytes.data(), bytes.data(), bytes.size());

    return std::string(reinterpret_cast<const char*>(outBytes.data()), outBytes.size()); // NOLINT
}

static std::string aesDecryptString(const std::string& string, const Auth::AesKey& aesKey)
{
    std::vector<byte> outBytes(string.size());

    C::CFB_Mode<C::AES>::Decryption decryption(static_cast<const byte*>(aesKey._aes),
        sizeof(aesKey._aes), static_cast<const byte*>(aesKey._iv));
    decryption.ProcessData(outBytes.data(), reinterpret_cast<const byte*>(string.data()), string.size()); // NOLINT

    return std::string(reinterpret_cast<const char*>(outBytes.data()), outBytes.size()); // NOLINT
}

static std::string aesEncryptString(const std::string& string, const Auth::AesKey& aesKey)
{
    auto inBytes = reinterpret_cast<const byte*>(string.data()); // NOLINT
    auto bytesSize = string.size();

    std::vector<byte> outBytes(bytesSize);

    C::CFB_Mode<C::AES>::Encryption encryption(static_cast<const byte*>(aesKey._aes),
        sizeof(aesKey._aes), static_cast<const byte*>(aesKey._iv));
    encryption.ProcessData(outBytes.data(), inBytes, bytesSize);

    return bytesToHex(outBytes);
}

static QJsonObject decodeAuthResponse(const Auth::AesKey& aesKey, const std::string& authResponseJsonString)
{
    QJsonParseError jsonError{};
    auto jsonDocument = QJsonDocument::fromJson(authResponseJsonString.data(), &jsonError);

    if(jsonError.error != QJsonParseError::ParseError::NoError)
        return {};

    auto jsonObject = jsonDocument.object();
    auto aesKeySignatureHex = jsonObject[QStringLiteral("signature")].toString().toStdString();
    auto aesKeySignature = hexToString(aesKeySignatureHex);

    C::RSA::PublicKey publicKey = loadPublicKey();

    bool sessionVerified = rsaVerifyAesKey(aesKey, aesKeySignature, publicKey);
    if(!sessionVerified)
        return {};

    auto encryptedPayload = hexToBytes(jsonObject[QStringLiteral("payload")].toString().toStdString());

    auto payload = aesDecryptBytes(encryptedPayload, aesKey);

    jsonDocument = QJsonDocument::fromJson(payload.data(), &jsonError);
    if(jsonError.error != QJsonParseError::ParseError::NoError)
        return {};

    jsonObject = jsonDocument.object();

    return jsonObject;
}

static std::string authRequest(const Auth::AesKey& aesKey, const QString& email, const QString& encryptedPassword)
{
    C::RSA::PublicKey publicKey = loadPublicKey();

    auto payloadJsonString = jsonObjectAsString(
    {
        {"email",    email},
        {"password", encryptedPassword},
        {"locale",   QLocale::system().name()},
        {"product",  PRODUCT_NAME},
        {"version",  VERSION}
    });
    auto encryptedPayload = aesEncryptString(payloadJsonString, aesKey);

    auto authReqJsonString = jsonObjectAsString(
    {
        {"key",     QString::fromStdString(rsaEncryptAesKey(aesKey, publicKey))},
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

    auto encrypted = hexToString(authToken.toStdString());

    std::string aesKeyAndEncryptedAuthToken;
    try
    {
        C::RSA::PublicKey publicKey = loadPublicKey();
        C::RSASSA_PKCS1v15_SHA_Verifier rsaVerifier(publicKey);
        C::StringSource ss(encrypted, true,
            new C::SignatureVerificationFilter(
                rsaVerifier, new C::StringSink(aesKeyAndEncryptedAuthToken),
                C::SignatureVerificationFilter::SIGNATURE_AT_BEGIN |
                C::SignatureVerificationFilter::PUT_MESSAGE |
                C::SignatureVerificationFilter::THROW_EXCEPTION
           ) // SignatureVerificationFilter
        ); // StringSource
        Q_UNUSED(ss);
    }
    catch(C::SignatureVerificationFilter::SignatureVerificationFailed&)
    {
        // If we get here, then someone is trying to manipulate the auth token
        return;
    }

    Auth::AesKey aesKey(aesKeyAndEncryptedAuthToken.c_str());

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
        for(const auto& v : value)
            _allowedPluginRegexps.emplace_back(v.toString());
    }
}

bool Auth::expired()
{
    parseAuthToken();

    auto now = static_cast<uint>(QDateTime::currentSecsSinceEpoch());
    auto approximatelyNow = now + 600; // Allow the system clock to be out by a little bit
    bool authenticated = (approximatelyNow >= _issueTime) && (now < _expiryTime);

    if(_authenticated != authenticated)
    {
        _authenticated = authenticated;
        emit stateChanged();
    }

    // If we're not authenticated now, the token has expired
    return !_authenticated;
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

    _aesKey = generateKey();
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
        _reply = _networkManager.post(request, multiPart);
        multiPart->setParent(_reply);
#endif
    });
}

void Auth::sendRequest(const QString& email, const QString& password)
{
    C::RSA::PublicKey publicKey = loadPublicKey();
    _encryptedPassword = QString::fromStdString((rsaEncryptString(password.toStdString(), publicKey)));

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
    _authenticated = false;
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
            _authenticated = true;
            emit stateChanged();
        }
        else
#endif

        if(_reply->error() == QNetworkReply::NetworkError::NoError)
        {
            std::string authResponse = _reply->readAll().toStdString();
            auto decodedRespose = decodeAuthResponse(_aesKey, authResponse);

            bool authenticated = decodedRespose.contains(QStringLiteral("authenticated")) &&
                decodedRespose[QStringLiteral("authenticated")].toBool() &&
                decodedRespose.contains(QStringLiteral("authToken"));

            if(_authenticated != authenticated)
            {
                _authenticated = authenticated;

                u::setPref("auth/password", u::pref("auth/rememberMe").toBool() ?
                    _encryptedPassword : QLatin1String(""));

                if(_authenticated)
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
        else
        {
            auto message = QStringLiteral("<b>NETWORK ERROR:</b> %1").arg(_reply->errorString());
            if(_message != message)
            {
                _message = message;
                emit messageChanged();
            }
        }

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
    _message = tr("Timed out waiting for a response from the authentication "
                  "server. Please check your internet connection and try again.");
    emit messageChanged();
    emit busyChanged();

    _reply->abort();
}
