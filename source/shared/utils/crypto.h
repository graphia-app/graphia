#ifndef CRYPTO_H
#define CRYPTO_H

#include <cstring>
#include <string>
#include <vector>
#include <cstddef>

namespace u
{
    struct AesKey
    {
        AesKey() = default;
        explicit AesKey(const char* bytes)
        {
            std::memcpy(_aes, &bytes[0],            sizeof(_aes));
            std::memcpy(_iv,  &bytes[sizeof(_aes)], sizeof(_iv));
        }

        unsigned char _aes[16] = {0};
        unsigned char _iv[16] = {0};
    };

    AesKey generateAesKey();

    std::string aesDecryptBytes(const std::vector<std::byte>& bytes, const AesKey& aesKey);
    std::string aesDecryptString(const std::string& string, const AesKey& aesKey);
    std::string aesEncryptString(const std::string& string, const AesKey& aesKey);

    bool rsaVerifySignature(const std::string& string, const std::string& signature,
        const std::string& publicKeyFileName, std::string* message = nullptr);
    bool rsaVerifySignature(const std::string& signaturePlusString,
        const std::string& publicKeyFileName, std::string* message = nullptr);

    std::string rsaEncryptString(const std::string& string,
        const std::string& publicKeyFileName);
} // namespace u

#endif // CRYPTO_H
