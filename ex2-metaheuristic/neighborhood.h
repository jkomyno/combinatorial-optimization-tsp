#pragma once

#include <algorithm>
#include <vector>

#include <shared/path_utils/PermutationPath.h>

#include "mutation.h"
#include "sampling.h"

namespace neighborhood {
    namespace detail {
        // Find the best neighbor of a given path using mutation strategies.
        // Get indexes is a closure (a function that returns a function) used to retrieve the pair
        // of mutation indexes.
        template <typename T, class GetIndexes>
        PermutationPath<T> find_best_neighbor(const PermutationPath<T>& path,
                                              GetIndexes&& get_indexes) noexcept {
            const size_t n = path.size();

            // Boolean comparator used for the PermutationPath<T>* objects.
            const auto comparator = [](auto path_a, auto path_b) -> bool {
                return path_a->cost() > path_b->cost();
            };

            PermutationPath<T> path_swap(path);
            PermutationPath<T> path_left_rotation(path);
            PermutationPath<T> path_right_rotation(path);
            PermutationPath<T> path_inversion(path);

            const auto get_indexes_partial = get_indexes();

            const auto [i_swap, j_swap] = get_indexes_partial();
            const auto [i_left_rotation, j_left_rotation] = get_indexes_partial();
            const auto [i_right_rotation, j_right_rotation] = get_indexes_partial();
            const auto [i_inversion, j_inversion] = get_indexes_partial();

            mutation::swap(path_swap, i_swap, j_swap);
            mutation::left_rotation(path_left_rotation, i_left_rotation, j_left_rotation);
            mutation::right_rotation(path_right_rotation, i_right_rotation, j_right_rotation);
            mutation::inversion(path_inversion, i_inversion, j_inversion);

            // clang-format off
            std::vector<PermutationPath<T>*> candidates{
                &path_swap,
				&path_left_rotation,
                &path_right_rotation,
                &path_inversion
            };
            // clang-format on

            auto it = std::min_element(candidates.cbegin(), candidates.cend(), comparator);
            const PermutationPath<T>* best_path_ptr = *it;
            return *best_path_ptr;
        }

        // Given a path and its best neighbor, it updates the path if its cost is higher than its
        // best neighbor's cost.
        template <typename T>
        void change_neighborhood(PermutationPath<T>& path, PermutationPath<T>&& best_neighbor,
                                 size_t& k) noexcept {
            if (best_neighbor.cost() < path.cost()) {
                // make a move
                path = std::move(best_neighbor);

                // initial neighborhood
                k = 0;
            } else {
                // next neighborhood
                ++k;
            }
        }
    }  // namespace detail

    template <typename T, class URBG>
    void variable_neighborhood_descent(PermutationPath<T>& path, URBG&& random_generator,
                                       size_t k_max = 3) noexcept {
        const size_t n = path.size();

        const auto get_indexes_dynamic = [&random_generator, n]() {
            return [&random_generator, n]() -> std::pair<size_t, size_t> {
                return sampling::sample_pair<true>(1, n - 1, random_generator);
            };
        };

        const auto get_indexes_static = [&random_generator, n]() {
            const auto ij = sampling::sample_pair<true>(1, n - 1, random_generator);
            return [ij]() -> std::pair<size_t, size_t> { return ij; };
        };

        size_t k = 0;
        while (k < k_max) {
            PermutationPath<T>&& best_neighbor(
                detail::find_best_neighbor(path, get_indexes_static));
            detail::change_neighborhood(path, std::move(best_neighbor), k);
        }
    }

    template <typename T>
    void complete_variable_neighborhood_search(PermutationPath<T>& path) noexcept {
        const size_t n = path.size();
        T best_cost = path.cost();

        for (size_t i = 0; i < n - 1; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                const auto get_indexes = [=]() {
                    return [=]() -> std::pair<size_t, size_t> { return {i, j}; };
                };
                auto&& best_neighbor = detail::find_best_neighbor(path, get_indexes);

                if (best_cost > best_neighbor.cost()) {
                    best_cost = best_neighbor.cost();
                    path = std::move(best_neighbor);
                }
            }
        }
    }

    template <typename T>
    void windowed_variable_neighborhood_search(PermutationPath<T>& path,
                                               std::pair<size_t, size_t>&& window) noexcept {
        const size_t n = path.size();
        T best_cost = path.cost();

        const auto [lb, ub] = window;

        for (size_t i = lb; i < ub - 1; ++i) {
            for (size_t j = i + 1; j < ub; ++j) {
                const auto get_indexes = [=]() {
                    return [=]() -> std::pair<size_t, size_t> { return {i, j}; };
                };
                auto&& best_neighbor = detail::find_best_neighbor(path, get_indexes);

                if (best_cost > best_neighbor.cost()) {
                    best_cost = best_neighbor.cost();
                    path = std::move(best_neighbor);
                }
            }
        }
    }
}  // namespace neighborhood