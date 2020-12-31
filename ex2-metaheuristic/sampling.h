#pragma once

#include <random>         // std::uniform_int_distribution
#include <unordered_set>  // std::unordered_set

namespace sampling {
    // Sample k distinct indexes from [0, ..., n-1], k < n.
    // It implements Robert Floyd's algorithm for sampling without replacement.
    // See: https://www.nowherenearithaca.com/2013/05/robert-floyds-tiny-and-beautiful.html
    template <class URBG>
    std::unordered_set<size_t> sample_indexes(size_t n, size_t k, URBG&& random) noexcept {
        using distr_t = std::uniform_int_distribution<size_t>;
        using param_t = typename distr_t::param_type;
        distr_t distribution;

        std::unordered_set<size_t> sample_indexes;
        sample_indexes.reserve(k);

        for (size_t i = n - k - 1; i < n; ++i) {
            const size_t v = distribution(random, param_t(0, i));

            // If v is new, add it. If v i already in sample_indexes, add i, which definitely isn't
            // in sample_indexes. In fact it's the first iteration of the loop that we could have
            // picked up a value that big.
            if (!sample_indexes.insert(v).second) {
                sample_indexes.insert(i);
            }
        }

        return sample_indexes;
    }

    // Sample k distinct elements from the elements in range [first, last].
    template <typename T, class RandomIt, class URBG>
    std::vector<T> sample_from_range(RandomIt first, RandomIt last, size_t k, URBG&& random) {
        size_t n = std::distance(first, last);

        std::vector<T> sample_values;
        sample_values.reserve(k);

        for (auto i : sample_indexes(n, k, random)) {
            const auto v = std::next(first, i);
            sample_values.emplace_back(*v);
        }

        return sample_values;
    }
}  // namespace sampling