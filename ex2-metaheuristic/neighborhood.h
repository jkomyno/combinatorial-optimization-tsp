#pragma once

#include <priority_queue/BinaryHeap.h>
#include <vector>

#include "PermutationPath.h"
#include "mutation.h"

namespace neighborhood {
    template <typename T>
    void variable_neighborhood_search(PermutationPath<T>& path) noexcept {
        const size_t n = path.size();
        T best_cost = path.cost();

        // Boolean comparator used for the PermutationPath<T>* objects in heap::BinaryHeap.
        const auto comparator = [](auto path_a, auto path_b) -> bool {
            return path_a->cost() > path_b->cost();
        };

        for (size_t i = 0; i < n - 1; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                PermutationPath<T> path_swap(path);
                PermutationPath<T> path_right_rotation(path);
                PermutationPath<T> path_inversion(path);

                mutation::swap(path_swap, i, j);
                mutation::right_rotation(path_right_rotation, i, j);
                mutation::inversion(path_inversion, i, j);

                // clang-format off
                std::vector<PermutationPath<T>*> candidates{
                    &path_swap,
                    &path_right_rotation,
                    &path_inversion
                };
                // clang-format on

                heap::BinaryHeap<PermutationPath<T>*, false> min_heap(std::move(candidates),
                                                                      comparator);

                if (best_cost > min_heap.top()->cost()) {
                    best_cost = min_heap.top()->cost();
                    path = std::move(*(min_heap.top()));
                }
            }
        }
    }
}  // namespace neighborhood