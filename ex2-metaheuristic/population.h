#pragma once

#include <algorithm>  // std::shuffle
#include <vector>     // std::vector

#include "PermutationPath.h"

namespace population {
    namespace detail {
        // Generate random feasible solutions until the population pool reaches size μ.
        // Each feasible solution is generated permutating a given seed path at random.
        template <typename T, class URBG>
        inline void generate_random(std::vector<PermutationPath<T>>& population_pool,
                                    const PermutationPath<T>& seed_path, size_t mu,
                                    URBG&& random_generator) noexcept {

            while (population_pool.size() < mu) {
                // create a new feasible solution and randomly shuffle it
                PermutationPath<T> candidate_path(seed_path);
                candidate_path.reset_cost();

                std::shuffle(candidate_path.begin() + 1, candidate_path.end(), random_generator);

                // add the new candidate path to the population pool
                population_pool.emplace_back(candidate_path);
            }
        }
    }  // namespace detail

    // Generate the initial population of feasible solutions until the population pool reaches size
    // μ. Each path in the population has size n, with cities represented as numbers in [0, n-1].
    // The population is composed of random permutations of a given heuristic path.
    template <bool include_heuristic, typename T, class URBG>
    std::vector<PermutationPath<T>> generate_initial(const PermutationPath<T>& heuristic_path,
                                                     size_t mu, size_t n,
                                                     URBG&& random_generator) noexcept {
        std::vector<PermutationPath<T>> population_pool;
        population_pool.reserve(mu);

        if constexpr (include_heuristic) {
            population_pool.emplace_back(heuristic_path);
        }

        // Generate random feasible solutions until the population pool reaches size μ.
        detail::generate_random(population_pool, heuristic_path, mu, random_generator);

        return population_pool;
    }
}  // namespace population