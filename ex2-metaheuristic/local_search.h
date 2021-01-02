#pragma once

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
}  // namespace local_search