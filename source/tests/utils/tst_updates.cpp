/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "build_defines.h"

#include "shared/updates/updates.h"
#include "shared/utils/crypto.h"
#include "shared/utils/json.h"
#include "shared/utils/string.h"

#include <cryptopp/rsa.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QByteArray>
#include <QString>
#include <QSysInfo>

#include <string>

using namespace Qt::Literals::StringLiterals;

namespace
{
// An update for the running OS
json update(const std::string& version, bool force = true, const std::string& targetVersionRegex = ".*")
{
    json update =
    {
        {"version", version},
        {"targetVersionRegex", targetVersionRegex},
        {"changeLog", "Changes in " + version},
        {"images", json::array({"image.png"})},
        {"payloads",
        {
            {QSysInfo::kernelType().toStdString(),
            {
                {"url", "https://example.com/" + version},
                {"installerFileName", "installer-" + version},
                {"installerChecksum", "checksum-" + version},
                {"command", "install " + version},
            }},
        }},
    };

    if(force)
        update["force"] = true;

    return update;
}

// QCOMPARE can't compare json values on every platform, so they're compared as strings
QString valueOf(const json& object, const char* key)
{
    if(!object.is_object() || !object.contains(key))
        return {};

    const auto& value = object[key];
    return QString::fromStdString(value.is_string() ? value.get<std::string>() : value.dump());
}
} // namespace

class TestUpdates : public QObject
{
    Q_OBJECT

    QTemporaryDir _dir;

    std::string path(const QString& fileName) const
    {
        return _dir.filePath(fileName).toStdString();
    }

    void writeFile(const QString& fileName, const std::string& contents)
    {
        QFile file(_dir.filePath(fileName));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray::fromStdString(contents));
    }

    // Writes a key pair to <name>.private.der and <name>.public.der
    void generateKeyPair(const QString& name)
    {
        CryptoPP::AutoSeededRandomPool rng;

        CryptoPP::InvertibleRSAFunction parameters;
        parameters.GenerateRandomWithKeySize(rng, 1024);

        const CryptoPP::RSA::PrivateKey privateKey(parameters);
        CryptoPP::RSA::PublicKey publicKey;
        publicKey.Initialize(parameters.GetModulus(), parameters.GetPublicExponent());

        std::string privateDer;
        CryptoPP::StringSink privateSink(privateDer);
        privateKey.Save(privateSink);
        writeFile(name + u".private.der"_s, privateDer);

        std::string publicDer;
        CryptoPP::StringSink publicSink(publicDer);
        publicKey.Save(publicSink);
        writeFile(name + u".public.der"_s, publicDer);
    }

    // A manifest of the given updates, signed with the test key
    QString manifest(const json& updates, bool hexEncoded = false, const json& extra = json::object())
    {
        auto updatesString = updates.dump();
        if(hexEncoded)
            updatesString = u::bytesToHex(updatesString);

        auto signature = u::rsaSignString(updatesString, path(u"test.private.der"_s));

        json manifest = {{"updates", updatesString}, {"signature", u::bytesToHex(signature)}};
        manifest.update(extra);

        return QString::fromStdString(manifest.dump());
    }

    json parse(const QString& manifest, QString* status = nullptr)
    {
        return updateStringToJson(manifest, status, path(u"test.public.der"_s));
    }

private slots:
    void initTestCase()
    {
        QVERIFY(_dir.isValid());
        generateKeyPair(u"test"_s);
        generateKeyPair(u"other"_s);
    }

    void update()
    {
        QString status;
        auto result = parse(manifest(json::array({::update("1.0")}), false, {{"status", "Some status"}}), &status);

        QCOMPARE(valueOf(result, "version"), u"1.0"_s);
        QCOMPARE(valueOf(result, "url"), u"https://example.com/1.0"_s);
        QCOMPARE(valueOf(result, "installerFileName"), u"installer-1.0"_s);
        QCOMPARE(valueOf(result, "installerChecksum"), u"checksum-1.0"_s);
        QCOMPARE(valueOf(result, "command"), u"install 1.0"_s);
        QCOMPARE(valueOf(result, "changeLog"), u"Changes in 1.0"_s);
        QCOMPARE(valueOf(result, "images"), uR"(["image.png"])"_s);
        QVERIFY(!result.contains("httpUserName"));
        QCOMPARE(status, u"Some status"_s);
    }

    void hexEncodedUpdates()
    {
        QCOMPARE(valueOf(parse(manifest(json::array({::update("1.0")}), true)), "version"), u"1.0"_s);
    }

    void httpCredentials()
    {
        auto withCredentials = ::update("1.0");
        auto& payload = withCredentials["payloads"][QSysInfo::kernelType().toStdString()];
        payload["httpUserName"] = "user";
        payload["httpPassword"] = "password";

        auto result = parse(manifest(json::array({withCredentials})));
        QCOMPARE(valueOf(result, "httpUserName"), u"user"_s);
        QCOMPARE(valueOf(result, "httpPassword"), u"password"_s);
    }

    void latestUpdate()
    {
        // Versions are compared numerically
        auto result = parse(manifest(json::array({::update("2.9"), ::update("2.10"), ::update("2.1")})));
        QCOMPARE(valueOf(result, "version"), u"2.10"_s);
    }

    void forcedUpdatesFirst()
    {
        // An update to the running version is kept, so its change log can be shown
        auto result = parse(manifest(json::array({::update(VERSION, false), ::update("0.1")})));
        QCOMPARE(valueOf(result, "version"), u"0.1"_s);

        result = parse(manifest(json::array({::update(VERSION, false)})));
        QCOMPARE(valueOf(result, "version"), QStringLiteral(VERSION));
    }

    void inapplicableUpdates()
    {
        // For other versions
        QVERIFY(parse(manifest(json::array({::update("1.0", true, "^not the running version$")}))).is_null());

        auto noRegex = ::update("1.0");
        noRegex.erase("targetVersionRegex");
        QVERIFY(parse(manifest(json::array({noRegex}))).is_null());

        // For other operating systems
        auto otherOs = ::update("1.0");
        otherOs["payloads"] = {{"some other OS", otherOs["payloads"].begin().value()}};
        QVERIFY(parse(manifest(json::array({otherOs}))).is_null());

        QVERIFY(parse(manifest(json::array())).is_null());

        // Only the update that applies is kept
        QCOMPARE(valueOf(parse(manifest(json::array({otherOs, ::update("1.1")}))), "version"), u"1.1"_s);
    }

    void invalidManifests_data()
    {
        QTest::addColumn<QString>("manifest");

        const auto valid = json::parse(manifest(json::array({::update("1.0")})).toStdString());

        auto withoutSignature = valid;
        withoutSignature.erase("signature");

        auto withoutUpdates = valid;
        withoutUpdates.erase("updates");

        auto signatureNotString = valid;
        signatureNotString["signature"] = 1;

        // The updates are changed after they were signed
        auto changedUpdates = valid;
        changedUpdates["updates"] = json::array({::update("6.0")}).dump();

        auto signedWithOtherKey = valid;
        signedWithOtherKey["signature"] = u::bytesToHex(u::rsaSignString(
            valid["updates"].get<std::string>(), path(u"other.private.der"_s)));

        auto updatesNotJson = valid;
        updatesNotJson["updates"] = "not JSON";
        updatesNotJson["signature"] = u::bytesToHex(u::rsaSignString("not JSON", path(u"test.private.der"_s)));

        QTest::newRow("not JSON") << u"not JSON"_s;
        QTest::newRow("no signature") << QString::fromStdString(withoutSignature.dump());
        QTest::newRow("no updates") << QString::fromStdString(withoutUpdates.dump());
        QTest::newRow("signature not a string") << QString::fromStdString(signatureNotString.dump());
        QTest::newRow("changed updates") << QString::fromStdString(changedUpdates.dump());
        QTest::newRow("signed with another key") << QString::fromStdString(signedWithOtherKey.dump());
        QTest::newRow("updates not JSON") << QString::fromStdString(updatesNotJson.dump());
    }

    void invalidManifests()
    {
        QFETCH(const QString, manifest);
        QVERIFY(parse(manifest).is_null());
    }

    // Only a manifest signed with Graphia's own key is accepted by default
    void builtInKey()
    {
        QVERIFY(updateStringToJson(manifest(json::array({::update("1.0")}))).is_null());
    }
};

QTEST_GUILESS_MAIN(TestUpdates)
#include "tst_updates.moc"
