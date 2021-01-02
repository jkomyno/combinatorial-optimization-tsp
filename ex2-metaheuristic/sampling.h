#pragma once

#include <algorithm>      // std::transform, std::generate
#include <iterator>       // std::back_inserter
#include <random>         // std::uniform_int_distribution, std::uniform_real_distribution
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

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

    // Compute probabilities sampled from a uniform distribution
    template <class RandomIt, class URBG>
    std::vector<double> sample_probabilities(RandomIt first, RandomIt last, URBG&& g) {
        size_t n = std::distance(first, last);

        std::vector<double> probabilities;
        probabilities.reserve(n);

        using distr_t = std::uniform_real_distribution<double>;
        using param_t = typename distr_t::param_type;
        distr_t distribution;

        std::transform(first, last, std::back_inserter(probabilities),
                       [&](auto &&) -> double { return distribution(g, param_t(0, 1)); });

        return probabilities;
    }

    template <class URBG>
    std::pair<size_t, size_t> sample_constrained_window(size_t low, size_t high, size_t delta_min,
                                                        size_t delta_max,
                                                        URBG&& random_generator) noexcept {

        using distr_t = std::uniform_real_distribution<double>;
        using param_t = typename distr_t::param_type;
        distr_t distribution;

        size_t space = high - low - delta_min;

        // extract two random numbers from [0,1), u_1 < u_2
        double u_1 = distribution(random_generator, param_t(0, 1));
        double u_2 = distribution(random_generator, param_t(0, 1));
        if (u_1 > u_2) {
            using std::swap;
            swap(u_1, u_2);
        }

        double x_1 = u_1 * space;
        double x_2 = u_2 * space;

        size_t a_1 = low + static_cast<int>(x_1);
        size_t a_2 = low + static_cast<int>(x_2) + delta_min;

        // clip upper bound
        if (a_2 > a_1 + delta_max) {
            a_2 = a_1 + delta_max;
        }

        return {a_1, a_2};
    }

    // Sample k elements with replacement from a data vector according to the given discrete range
    // of probabilities.
    template <typename T, class RandomIt, class URBG>
    std::vector<T> sample_from_probabilities(const std::vector<T>& data, size_t k, RandomIt first,
                                             RandomIt last, URBG&& random_generator) noexcept {
        std::discrete_distribution<size_t> distribution(first, last);

        std::vector<size_t> indexes(k);
        std::generate(indexes.begin(), indexes.end(),
                      [&]() { return distribution(random_generator); });

        std::vector<T> selection;
        selection.reserve(k);

        std::transform(indexes.begin(), indexes.end(), std::back_inserter(selection),
                       [&data](size_t index) { return data[size_t]; });

        return selection;
    }
}  // namespace sampling