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

#include "shared/utils/container.h"
#include "shared/utils/crypto.h"
#include "shared/utils/string.h"

#include <cryptopp/rsa.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QByteArray>
#include <QString>

#include <algorithm>
#include <array>
#include <cstring>
#include <string>

using namespace Qt::Literals::StringLiterals;

class TestCrypto : public QObject
{
    Q_OBJECT

    QTemporaryDir _dir;

    std::string path(const QString& fileName) const
    {
        return _dir.filePath(fileName).toStdString();
    }

    void writeFile(const QString& fileName, const QByteArray& contents)
    {
        QFile file(_dir.filePath(fileName));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(contents);
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
        writeFile(name + u".private.der"_s, QByteArray::fromStdString(privateDer));

        std::string publicDer;
        CryptoPP::StringSink publicSink(publicDer);
        publicKey.Save(publicSink);
        writeFile(name + u".public.der"_s, QByteArray::fromStdString(publicDer));
    }

    QByteArray readFile(const QString& fileName)
    {
        QFile file(_dir.filePath(fileName));
        if(!file.open(QIODevice::ReadOnly))
            return {};

        return file.readAll();
    }

private slots:
    void initTestCase()
    {
        QVERIFY(_dir.isValid());

        generateKeyPair(u"a"_s);
        generateKeyPair(u"b"_s);

        // The private key, as PEM
        auto base64 = readFile(u"a.private.der"_s).toBase64();
        QByteArray pem = "-----BEGIN PRIVATE KEY-----\n";
        for(qsizetype i = 0; i < base64.size(); i += 64)
            pem += base64.mid(i, 64) + "\n";
        pem += "-----END PRIVATE KEY-----\n";
        writeFile(u"a.private.pem"_s, pem);

        writeFile(u"garbage.der"_s, "this is not a key");
    }

    void signAndVerify()
    {
        const std::string string = "Some text to sign";

        auto signature = u::rsaSignString(string, path(u"a.private.der"_s));
        QVERIFY(!signature.empty());

        std::string message;
        QVERIFY(u::rsaVerifySignature(string, signature, path(u"a.public.der"_s), &message));
        QCOMPARE(message, string);

        // The signature followed by the string
        message.clear();
        QVERIFY(u::rsaVerifySignature(signature + string, path(u"a.public.der"_s), &message));
        QCOMPARE(message, string);

        // PKCS #1 v1.5 signatures are deterministic, so signing with the PEM
        // version of the same key gives the same signature
        QCOMPARE(u::rsaSignString(string, path(u"a.private.pem"_s)), signature);

        // As NativeUtils::rsaSignatureForString does, for the updates
        auto hexSignature = u::bytesToHex(signature);
        QCOMPARE(hexSignature.size(), 2 * signature.size());
        QVERIFY(u::isHex(hexSignature));
        QCOMPARE(u::hexToString(hexSignature), signature);
    }

    void verifyFailures()
    {
        const std::string string = "Some text to sign";
        auto signature = u::rsaSignString(string, path(u"a.private.der"_s));

        std::string message = "unchanged";

        // Changed text
        QVERIFY(!u::rsaVerifySignature("Some text to sigN", signature, path(u"a.public.der"_s), &message));
        QCOMPARE(message, "unchanged");

        // Changed signature
        auto changedSignature = signature;
        changedSignature.back() = static_cast<char>(changedSignature.back() ^ 1);
        QVERIFY(!u::rsaVerifySignature(string, changedSignature, path(u"a.public.der"_s)));

        // Missing or truncated signatures
        QVERIFY(!u::rsaVerifySignature(string, {}, path(u"a.public.der"_s)));
        QVERIFY(!u::rsaVerifySignature(string, signature.substr(0, 10), path(u"a.public.der"_s)));
        QVERIFY(!u::rsaVerifySignature(std::string{}, path(u"a.public.der"_s)));

        // The wrong key
        QVERIFY(!u::rsaVerifySignature(string, signature, path(u"b.public.der"_s)));

        // No key
        QVERIFY(!u::rsaVerifySignature(string, signature, path(u"missing.der"_s)));

        // Not a key
        QVERIFY(!u::rsaVerifySignature(string, signature, path(u"garbage.der"_s)));

        // A private key isn't a public key
        QVERIFY(!u::rsaVerifySignature(string, signature, path(u"a.private.der"_s)));
    }

    void signFailures()
    {
        QVERIFY(u::rsaSignString("text", path(u"missing.der"_s)).empty());
        QVERIFY(u::rsaSignString("text", path(u"garbage.der"_s)).empty());

        // A public key isn't a private key
        QVERIFY(u::rsaSignString("text", path(u"a.public.der"_s)).empty());
    }

    void rsaEncrypt()
    {
        auto a = u::rsaEncryptString("secret", path(u"a.public.der"_s));
        auto b = u::rsaEncryptString("secret", path(u"a.public.der"_s));

        // As hex, a byte per 8 bits of the key
        QVERIFY(u::isHex(a));
        QCOMPARE(a.size(), 2u * (1024u / 8u));

        // OAEP padding is random
        QVERIFY(a != b);
    }

    void aes()
    {
        const auto key = u::generateAesKey();
        const auto otherKey = u::generateAesKey();

        QVERIFY(std::memcmp(key._aes, otherKey._aes, sizeof(key._aes)) != 0);
        QVERIFY(std::memcmp(key._iv, otherKey._iv, sizeof(key._iv)) != 0);

        // Encrypting gives hex, while decrypting takes bytes
        const std::string string = "Some text that is longer than a single AES block";
        auto encrypted = u::aesEncryptString(string, key);
        QVERIFY(u::isHex(encrypted));

        // CFB mode needs no padding
        QCOMPARE(encrypted.size(), 2 * string.size());

        QCOMPARE(u::aesDecryptBytes(u::hexToBytes(encrypted), key), string);
        QCOMPARE(u::aesDecryptString(u::hexToString(encrypted), key), string);
        QVERIFY(u::aesDecryptString(u::hexToString(encrypted), otherKey) != string);
    }

    void aesKeyFromBytes()
    {
        std::array<char, 32> bytes{};
        u::iota(bytes, char{0});

        // The key is followed by the initialisation vector
        const u::AesKey key(bytes.data());
        QCOMPARE(key._aes[0], 0);
        QCOMPARE(key._aes[15], 15);
        QCOMPARE(key._iv[0], 16);
        QCOMPARE(key._iv[15], 31);

        const std::string string = "text";
        QCOMPARE(u::aesDecryptString(u::hexToString(u::aesEncryptString(string, key)), u::AesKey(bytes.data())), string);
    }
};

QTEST_GUILESS_MAIN(TestCrypto)
#include "tst_crypto.moc"
