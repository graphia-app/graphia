/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef IATTRIBUTERANGE_H
#define IATTRIBUTERANGE_H

// In case windows.h has been included prior to this file
#undef min
#undef max

class IAttribute;

template<typename T>
class IAttributeRange
{
public:
    IAttributeRange() = default;
    IAttributeRange(const IAttributeRange&) = default;
    IAttributeRange(IAttributeRange&&) noexcept = default;
    IAttributeRange& operator=(const IAttributeRange&) = default;
    IAttributeRange& operator=(IAttributeRange&&) noexcept = default;

    virtual ~IAttributeRange() = default;

    virtual bool hasMin() const = 0;
    virtual bool hasMax() const = 0;
    virtual bool hasRange() const = 0;

    virtual T min() const = 0;
    virtual T max() const = 0;
    virtual IAttribute& setMin(T) = 0;
    virtual IAttribute& setMax(T) = 0;

    bool valueInRange(T value) const
    {
        if(hasMin() && value < min())
            return false;

        if(hasMax() && value > max())
            return false;

        return true;
    }
};

#endif // IATTRIBUTERANGE_H
