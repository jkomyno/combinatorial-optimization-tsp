#pragma once

#include <random>         // std::uniform_int_distribution
#include <unordered_set>  // std::unordered_set

#include "utils.h"

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

        for (size_t i = n - k; i < n; ++i) {
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

    template <bool Sort, class URBG>
    std::pair<size_t, size_t> sample_pair(size_t n, URBG&& random) noexcept {
        std::unordered_set<size_t> indexes_set(sample_indexes(n, 2, random));

        size_t i = utils::pop(indexes_set);
        size_t j = utils::pop(indexes_set);

        if constexpr (Sort) {
            if (j < i) {
                using std::swap;
                swap(i, j);
            }
        }

        return {i, j};
    }

    // Sample k distinct elements from the elements in range [first, last].
    // Before picking the elements, apply the unary function map_f.
    template <class RandomIt, class URBG, class UnaryF>
    auto sample_from_range(RandomIt first, RandomIt last, size_t k, URBG&& random, UnaryF&& map_f) {
        using result_t = decltype(map_f(*first));

        size_t n = std::distance(first, last);

        std::vector<result_t> sample_values;
        sample_values.reserve(k);

        for (auto i : sample_indexes(n, k, random)) {
            const auto v = std::next(first, i);
            sample_values.emplace_back(map_f(*v));
        }

        return sample_values;
    }

    // Sample k distinct elements from the elements in range [first, last].
    template <class RandomIt, class URBG>
    auto sample_from_range(RandomIt first, RandomIt last, size_t k, URBG&& random) {
        const auto id = [](const auto& x) { return x; };
        return sample_from_range<RandomIt, URBG>(first, last, k, random, id);
    }
}  // namespace sampling