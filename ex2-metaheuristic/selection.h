#pragma once

#include <algorithm>  // std::accumulate
#include <cmath>      // std::exp
#include <vector>     // std::vector

#include <priority_queue/BinaryHeap.h>

#include "PermutationPath.h"
#include "sampling.h"

namespace selection {
    // Tournament selection implementation. It selects λ members from a pool of individuals.
    template <typename T, class URBG>
    std::vector<PermutationPath<T>> tournament(std::vector<PermutationPath<T>>& population_pool,
                                               size_t lambda, size_t k, URBG&& random_generator) {
        std::vector<PermutationPath<T>> selection_pool;
        selection_pool.reserve(lambda);

        // Boolean comparator used for the PermutationPath<T>* objects in heap::BinaryHeap.
        const auto comparator = [](auto path_a, auto path_b) -> bool {
            return path_a->cost() > path_b->cost();
        };

        // Map an element to the pointer to that element
        const auto to_pointer = [](auto& x) { return &x; };

        while (selection_pool.size() < lambda) {
            // 1) pick k random individuals from k distinct indexes with uniform probability, with
            // replacement.
            // 2) compare these k individuals and select the best.
            // 3) add the best to the mating pool.
            // 4) continue until λ parents have been selected

            auto candidates(sampling::sample_from_range(
                population_pool.begin(), population_pool.end(), k, random_generator, to_pointer));

            heap::BinaryHeap<PermutationPath<T>*> min_heap(std::move(candidates), comparator);

            // The best candidate has the minimum cost among the k candidates considered in the
            // current tournament extraction.
            PermutationPath<T>* best_candidate = min_heap.top();

            selection_pool.push_back(*best_candidate);
        }

        return selection_pool;
    }

    // Exponential ranking selection implementation. It selects λ members from a pool of
    // individuals.
    template <typename T, class URBG>
    std::vector<PermutationPath<T>> ranking(std::vector<PermutationPath<T>>& population_pool,
                                            size_t lambda, URBG&& random_generator) {
        // The most expensive path has rank 0, the less expensive has rank mu-1
        std::sort(
            population_pool.begin(), population_pool.end(),
            [](auto&& path_a, auto&& path_b) -> bool { return path_a.cost() > path_b.cost(); });

        const size_t mu = population_pool.size();

        /**
         * Compute exponential ranking probabilities
         */

        std::vector<double> probabilities_by_rank(mu);
        for (size_t rank = 0; rank < mu; ++rank) {
            probabilities_by_rank[rank] = (1.0 - std::exp(-rank));
        }

        // c is the normalization factor
        double c = std::accumulate(probabilities_by_rank.begin(), probabilities_by_rank.end(), 0.0);

        for (double& probabilities : probabilities_by_rank) {
            probabilities /= c;
        }

        /**
         * Select from population pool according to the ranking probabilities
         */

        return sampling::sample_from_probabilities(population_pool, lambda,
                                                   probabilities_by_rank.cbegin(),
                                                   probabilities_by_rank.cend(), random_generator);
    }
}  // namespace selection