/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "crypto.h"

#include "shared/utils/string.h"

#include <cryptopp/rsa.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>

#include <QFile>

u::AesKey u::generateAesKey()
{
    AesKey key;

    CryptoPP::AutoSeededRandomPool rng;

    rng.GenerateBlock(reinterpret_cast<CryptoPP::byte*>(key._aes), sizeof(key._aes));
    rng.GenerateBlock(reinterpret_cast<CryptoPP::byte*>(key._iv), sizeof(key._iv));

    return key;
}

std::string u::aesDecryptBytes(const std::vector<std::byte>& bytes, const u::AesKey& aesKey)
{
    std::vector<CryptoPP::byte> outBytes(bytes.size());

    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryption(reinterpret_cast<const CryptoPP::byte*>(aesKey._aes),
        sizeof(aesKey._aes), reinterpret_cast<const CryptoPP::byte*>(aesKey._iv));
    decryption.ProcessData(outBytes.data(), reinterpret_cast<const CryptoPP::byte*>(bytes.data()), bytes.size());

    return {reinterpret_cast<const char*>(outBytes.data()), outBytes.size()};
}

std::string u::aesDecryptString(const std::string& string, const u::AesKey& aesKey)
{
    std::vector<CryptoPP::byte> outBytes(string.size());

    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryption(reinterpret_cast<const CryptoPP::byte*>(aesKey._aes),
        sizeof(aesKey._aes), reinterpret_cast<const CryptoPP::byte*>(aesKey._iv));
    decryption.ProcessData(outBytes.data(), reinterpret_cast<const CryptoPP::byte*>(string.data()), string.size());

    return {reinterpret_cast<const char*>(outBytes.data()), outBytes.size()};
}

std::string u::aesEncryptString(const std::string& string, const u::AesKey& aesKey)
{
    const auto* inBytes = reinterpret_cast<const CryptoPP::byte*>(string.data());
    auto bytesSize = string.size();

    std::vector<CryptoPP::byte> outBytes(bytesSize);

    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryption(reinterpret_cast<const CryptoPP::byte*>(aesKey._aes),
        sizeof(aesKey._aes), reinterpret_cast<const CryptoPP::byte*>(aesKey._iv));
    encryption.ProcessData(outBytes.data(), inBytes, bytesSize);

    return u::bytesToHex(outBytes);
}

static QByteArray decodeFromPem(const QByteArray& source)
{
    QString text = QString::fromUtf8(source);

    const auto* header = "-----BEGIN PRIVATE KEY-----";
    const auto* footer = "-----END PRIVATE KEY-----";

    // If it's not PEM, just return it
    if(!text.contains(header) || !text.contains(footer))
        return source;

    // Get the raw base64 encoded string
    text = text.remove(header);
    text = text.remove(footer);
    text = text.simplified();
    text = text.remove(' ');

    return QByteArray::fromBase64(text.toUtf8());
}

template<typename Key>
Key loadKey(const std::string& fileName)
{
    QFile file(QString::fromStdString(fileName));
    if(!file.open(QIODevice::ReadOnly))
        return {};

    auto byteArray = file.readAll();
    file.close();

    byteArray = decodeFromPem(byteArray);

    CryptoPP::ArraySource arraySource(reinterpret_cast<const CryptoPP::byte*>(byteArray.constData()),
        static_cast<size_t>(byteArray.size()), true);

    Key key;
    key.Load(arraySource);

    return key;
}

std::string u::rsaSignString(const std::string& string, const std::string& privateKeyFileName)
{
    std::string signature;

    try
    {
        auto privateKey = loadKey<CryptoPP::RSA::PrivateKey>(privateKeyFileName);

        CryptoPP::AutoSeededRandomPool rng;
        const CryptoPP::RSASSA_PKCS1v15_SHA_Signer signer(privateKey);

        auto* sink = new CryptoPP::StringSink(signature);
        auto* filter = new CryptoPP::SignerFilter(rng, signer, sink);

        const CryptoPP::StringSource ss(string, true, filter);
        Q_UNUSED(ss);
    }
    catch(std::exception&)
    {
        return {};
    }

    return signature;
}

bool u::rsaVerifySignature(const std::string& string, const std::string& signature,
    const std::string& publicKeyFileName, std::string* message)
{
    return rsaVerifySignature(signature + string, publicKeyFileName, message);
}

bool u::rsaVerifySignature(const std::string& signaturePlusString,
    const std::string& publicKeyFileName, std::string* message)
{
    auto publicKey = loadKey<CryptoPP::RSA::PublicKey>(publicKeyFileName);

    const CryptoPP::RSASSA_PKCS1v15_SHA_Verifier rsaVerifier(publicKey);
    std::string recoveredMessage;

    try
    {
        auto* sink = new CryptoPP::StringSink(recoveredMessage);
        auto* filter = new CryptoPP::SignatureVerificationFilter(rsaVerifier, sink,
            CryptoPP::SignatureVerificationFilter::SIGNATURE_AT_BEGIN |
            CryptoPP::SignatureVerificationFilter::PUT_MESSAGE |
            CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION);

        const CryptoPP::StringSource ss(signaturePlusString, true, filter);
        Q_UNUSED(ss);
    }
    catch(std::exception&)
    {
        return false;
    }

    if(message != nullptr)
        *message = recoveredMessage;

    return true;
}

std::string u::rsaEncryptString(const std::string& string, const std::string& publicKeyFileName)
{
    auto publicKey = loadKey<CryptoPP::RSA::PublicKey>(publicKeyFileName);
    const CryptoPP::RSAES_OAEP_SHA_Encryptor rsaEncryptor(publicKey);

    CryptoPP::AutoSeededRandomPool rng;
    std::vector<CryptoPP::byte> cipher(rsaEncryptor.FixedCiphertextLength());

    auto* sink = new CryptoPP::ArraySink(cipher.data(), cipher.size());
    auto* filter = new CryptoPP::PK_EncryptorFilter(rng, rsaEncryptor, sink);

    const CryptoPP::StringSource ss(string, true, filter);
    Q_UNUSED(ss);

    return u::bytesToHex(cipher);
}
