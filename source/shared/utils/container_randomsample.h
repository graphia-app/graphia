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

    std::default_random_engine dre(static_cast<int>(container.front()));

    for(size_t i = 0; i < numSamples; i++)
    {
        int high = static_cast<int>(sample.size() - i) - 1;
        std::uniform_int_distribution uid(0, high);
        std::swap(sample[static_cast<size_type>(i)],
            sample[static_cast<size_type>(i) + static_cast<size_type>(uid(dre))]);
    }

    sample.resize(static_cast<size_type>(numSamples));

    return sample;
}
} // namespace u

#endif // CONTAINER_RANDOMSAMPLE_H
