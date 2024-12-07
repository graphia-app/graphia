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

    std::string rsaSignString(const std::string& string,
        const std::string& privateKeyFileName);

    bool rsaVerifySignature(const std::string& string, const std::string& signature,
        const std::string& publicKeyFileName, std::string* message = nullptr);
    bool rsaVerifySignature(const std::string& signaturePlusString,
        const std::string& publicKeyFileName, std::string* message = nullptr);

    std::string rsaEncryptString(const std::string& string,
        const std::string& publicKeyFileName);
} // namespace u

#endif // CRYPTO_H
