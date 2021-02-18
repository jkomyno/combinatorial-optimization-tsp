#pragma once

#include <shared/path_utils/PermutationPath.h>

#include <algorithm>
#include <vector>

namespace statistics {
    template <typename T>
    T sum_cost(const std::vector<PermutationPath<T>>& population_pool) noexcept {
        return std::accumulate(
            population_pool.cbegin(), population_pool.cend(), T(0.0),
            [](T cost, const PermutationPath<T>& path) -> T { return cost + path.cost(); });
    }

    template <typename T>
    T average_cost(const std::vector<PermutationPath<T>>& population_pool) noexcept {
        return sum_cost(population_pool) / static_cast<T>(population_pool.size());
    }

}  // namespace statistics