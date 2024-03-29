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

#ifndef CONTAINER_RANDOMSAMPLE_H
#define CONTAINER_RANDOMSAMPLE_H

#include <random>

namespace u
{
template<typename T, template<typename, typename...> typename C, typename... Args>
C<T, Args...> randomSample(const C<T, Args...>& container, size_t numSamples)
{
    using size_type = typename C<T>::size_type;

    if(container.empty() || numSamples > static_cast<size_t>(container.size()))
        return container;

    auto sample = container;

    using result_type = std::default_random_engine::result_type;
    std::default_random_engine dre(static_cast<result_type>(container.front()));

    for(size_type i = 0; i < static_cast<size_type>(numSamples); i++)
    {
        const int high = static_cast<int>(sample.size() - i) - 1;
        std::uniform_int_distribution uid(0, high);
        std::swap(sample[static_cast<size_type>(i)],
            sample[static_cast<size_type>(i) + static_cast<size_type>(uid(dre))]);
    }

    sample.resize(static_cast<size_type>(numSamples));

    return sample;
}
} // namespace u

#endif // CONTAINER_RANDOMSAMPLE_H
