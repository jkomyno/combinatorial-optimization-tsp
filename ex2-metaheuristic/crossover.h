#pragma once

#include <algorithm>      // std::rotate
#include <iterator>       // std::begin, std::end
#include <unordered_set>  // std::unordered_set
#include <utility>        // std::pair
#include <vector>         // std::vector

#include "PermutationPath.h"
#include "sampling.h"

namespace crossover {
    namespace detail {
        // Order crossover (OX).
        // The pair of parent indexes must be sorted in ascending order.
        template <typename T>
        std::pair<PermutationPath<T>, PermutationPath<T>> order(
            PermutationPath<T>& parent_1, PermutationPath<T>& parent_2,
            std::pair<size_t, size_t> indexes_parent) noexcept {

            const size_t N = parent_1.size();

            const auto [x, y] = indexes_parent;

            std::unordered_set<size_t> inserted_in_offspring_1;
            inserted_in_offspring_1.reserve(parent_1.size());

            std::unordered_set<size_t> inserted_in_offspring_2;
            inserted_in_offspring_2.reserve(parent_2.size());

            inserted_in_offspring_1.insert(std::next(parent_1.begin(), x),
                                           std::next(parent_1.begin(), y + 1));

            inserted_in_offspring_2.insert(std::next(parent_2.begin(), x),
                                           std::next(parent_2.begin(), y + 1));

            PermutationPath<T> offspring_1(parent_1);
            offspring_1.reset_cost();

            PermutationPath<T> offspring_2(parent_2);
            offspring_2.reset_cost();

            // Rotate parent vectors to put the cut at the end of each parent.
            // E.g. [7 1 6 2 | 5 8 9 | 3 4] -> [3 4 7 1 6 2 | 5 8 9]
            std::rotate(parent_1.begin(), std::next(parent_1.begin(), y + 1), parent_1.end());
            std::rotate(parent_2.begin(), std::next(parent_2.begin(), y + 1), parent_2.end());

            size_t i, j;

            auto order_helper =
                [&i](size_t value, PermutationPath<T>& offspring,
                     std::unordered_set<size_t>& inserted_in_offspring) mutable -> void {
                const bool can_insert = inserted_in_offspring.count(value) == 0;
                if (can_insert) {
                    offspring[i] = value;
                    ++i;
                    inserted_in_offspring.insert(value);
                }
            };

            i = 0;
            j = 0;
            for (; i < x; ++j) {
                T value = parent_2[j];
                order_helper(value, offspring_1, inserted_in_offspring_1);
            }

            i = y + 1;
            for (; i < N && j < N; ++j) {
                T value = parent_2[j];
                order_helper(value, offspring_1, inserted_in_offspring_1);
            }

            i = 0;
            j = 0;
            for (; i < x; ++j) {
                T value = parent_1[j];
                order_helper(value, offspring_2, inserted_in_offspring_2);
            }

            i = y + 1;
            for (; i < N && j < N; ++j) {
                T value = parent_1[j];
                order_helper(value, offspring_2, inserted_in_offspring_2);
            }

            // Reset parent rotation
            std::rotate(parent_1.begin(), std::next(parent_1.begin(), N - y - 1), parent_1.end());
            std::rotate(parent_2.begin(), std::next(parent_2.begin(), N - y - 1), parent_2.end());

            return {PermutationPath<T>(std::move(offspring_1)),
                    PermutationPath<T>(std::move(offspring_2))};
        }

        // Variation of Davis' order crossover (OX) with two cut pairs rather than just one.
        // parent_1 and parent_2 might be manipulated during the execution of this function,
        // but at the end of it they're guaranteed to be reordered to the initial configuration.
        // The pairs of parent indexes must be sorted in ascending order.
        // See: https://www.redalyc.org/pdf/2652/265219618002.pdf
        template <typename T>
        std::pair<PermutationPath<T>, PermutationPath<T>> order_alt(
            PermutationPath<T>& parent_1, PermutationPath<T>& parent_2,
            std::pair<size_t, size_t> indexes_parent_1,
            std::pair<size_t, size_t> indexes_parent_2) noexcept {

            const size_t N = parent_1.size();

            const auto [x, y] = indexes_parent_1;
            const auto [w, z] = indexes_parent_2;

            std::unordered_set<size_t> inserted_in_offspring_1;
            inserted_in_offspring_1.reserve(parent_1.size());
            std::unordered_set<size_t> inserted_in_offspring_2;
            inserted_in_offspring_2.reserve(parent_2.size());

            inserted_in_offspring_1.insert(std::next(parent_1.begin(), x),
                                           std::next(parent_1.begin(), y + 1));

            inserted_in_offspring_2.insert(std::next(parent_2.begin(), w),
                                           std::next(parent_2.begin(), z + 1));

            PermutationPath<T> offspring_1(parent_1);
            offspring_1.reset_cost();
            PermutationPath<T> offspring_2(parent_2);
            offspring_2.reset_cost();

            // Rotate parent vectors to put the cut at the end of each parent.
            // E.g. [7 1 6 2 | 5 8 9 | 3 4] -> [3 4 7 1 6 2 | 5 8 9]
            std::rotate(parent_1.begin(), std::next(parent_1.begin(), y + 1), parent_1.end());
            std::rotate(parent_2.begin(), std::next(parent_2.begin(), z + 1), parent_2.end());

            size_t i, j;

            auto order_helper =
                [&i](size_t value, PermutationPath<T>& offspring,
                     std::unordered_set<size_t>& inserted_in_offspring) mutable -> void {
                const bool can_insert = inserted_in_offspring.count(value) == 0;
                if (can_insert) {
                    offspring[i] = value;
                    ++i;
                    inserted_in_offspring.insert(value);
                }
            };

            i = 0;
            j = 0;
            for (; i < x; ++j) {
                T value = parent_2[j];
                order_helper(value, offspring_1, inserted_in_offspring_1);
            }

            i = y + 1;
            for (; i < N && j < N; ++j) {
                T value = parent_2[j];
                order_helper(value, offspring_1, inserted_in_offspring_1);
            }

            i = 0;
            j = 0;
            for (; i < w; ++j) {
                T value = parent_1[j];
                order_helper(value, offspring_2, inserted_in_offspring_2);
            }

            i = z + 1;
            for (; i < N && j < N; ++j) {
                T value = parent_1[j];
                order_helper(value, offspring_2, inserted_in_offspring_2);
            }

            // Reset parent rotation
            std::rotate(parent_1.begin(), std::next(parent_1.begin(), N - y - 1), parent_1.end());
            std::rotate(parent_2.begin(), std::next(parent_2.begin(), N - z - 1), parent_2.end());

            return {PermutationPath<T>(std::move(offspring_1)),
                    PermutationPath<T>(std::move(offspring_2))};
        }
    }  // namespace detail

    template <typename T, class URBG>
    std::pair<PermutationPath<T>, PermutationPath<T>> order(PermutationPath<T>& parent_1,
                                                            PermutationPath<T>& parent_2, size_t n,
                                                            URBG&& random_generator) noexcept {
        std::pair<size_t, size_t> cut_indexes(sampling::sample_pair<true>(n, random_generator));
        return detail::order(parent_1, parent_2, cut_indexes);
    }

    template <typename T, class URBG>
    std::pair<PermutationPath<T>, PermutationPath<T>> order_alt(PermutationPath<T>& parent_1,
                                                                PermutationPath<T>& parent_2,
                                                                size_t n,
                                                                URBG&& random_generator) noexcept {
        std::pair<size_t, size_t> cut_indexes_1(sampling::sample_pair<true>(n, random_generator));
        std::pair<size_t, size_t> cut_indexes_2(sampling::sample_pair<true>(n, random_generator));

        return detail::order_alt(parent_1, parent_2, cut_indexes_1, cut_indexes_2);
    }
}  // namespace crossover
