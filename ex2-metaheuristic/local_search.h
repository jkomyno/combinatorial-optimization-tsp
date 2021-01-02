#pragma once

#include <cmath>   // std::log2
#include <vector>  // std::vector

#include "PermutationPath.h"
#include "neighborhood.h"

namespace local_search {
    // Apply variable neighborhood search to all the sequential pairs of indexes (i, j), i < j
    // of each path of the given population pool
    template <typename T>
    void improve_generation_complete(std::vector<PermutationPath<T>>& population_pool) {
        for (auto& path : population_pool) {
            neighborhood::variable_neighborhood_search(path);
        }
    }

    // Apply variable neighborhood search to a random subset of indexes pair induced by a
    // constrained random window for each path of the given population pool.
    // The window constraints have been selected to cover almost the entire path when n is small,
    // and increasingly smaller subpaths when n grows. It's less effective than
    // ::improve_generation_complete but much faster.
    // E.g.:
    // - n: 14 => window deltas: [2, 8]
    template <typename T, class URBG>
    void improve_generation_greedy(std::vector<PermutationPath<T>>& population_pool,
                                   URBG&& random_generator) {
        const size_t n = population_pool[0].size();
        const size_t low = 1;
        const size_t high = n - 1;

        const double log_threshold = std::log2(2.0 * n / 5.0);

        const size_t delta_min = static_cast<size_t>(log_threshold);
        const size_t delta_max = static_cast<size_t>(3.5 * log_threshold);

        for (auto& path : population_pool) {
            std::pair<size_t, size_t> window = sampling::sample_constrained_window(
                low, high, delta_min, delta_max, random_generator);

            neighborhood::windowed_variable_neighborhood_search(path, std::move(window));
        }
    }
}  // namespace local_search