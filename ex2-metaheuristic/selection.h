#pragma once

#include <vector>  // std::vector

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
}  // namespace selection