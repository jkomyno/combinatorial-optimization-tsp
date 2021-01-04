#pragma once

#include <algorithm>  // std::accumulate
#include <cmath>      // std::exp
#include <vector>     // std::vector

#include <priority_queue/BinaryHeap.h>

#include "PermutationPath.h"
#include "sampling.h"

namespace selection {
    namespace parent {
        // Tournament selection implementation. It selects λ members from a pool of individuals.
        template <typename T, class URBG>
        std::vector<PermutationPath<T>> tournament(std::vector<PermutationPath<T>>& population_pool,
                                                   size_t lambda, size_t k,
                                                   URBG&& random_generator) noexcept {
            std::vector<PermutationPath<T>> selection_pool;
            selection_pool.reserve(lambda);

            // Boolean comparator used for the PermutationPath<T>* objects in heap::BinaryHeap.
            const auto comparator = [](auto path_a, auto path_b) -> bool {
                return path_a->cost() > path_b->cost();
            };

            // Map an element to the pointer to that element
            const auto to_pointer = [](auto& x) { return &x; };

            while (selection_pool.size() < lambda) {
                // 1) pick k random individuals from k distinct indexes with uniform probability,
                // with replacement. 2) compare these k individuals and select the best. 3) add the
                // best to the mating pool. 4) continue until λ parents have been selected

                auto candidates(sampling::sample_from_range(population_pool.begin(),
                                                            population_pool.end(), k,
                                                            random_generator, to_pointer));

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
                                                size_t lambda, URBG&& random_generator) noexcept {
            // The most expensive path has rank 0, the less expensive has rank mu-1
            std::sort(
                population_pool.begin(), population_pool.end(),
                [](auto&& path_a, auto&& path_b) -> bool { return path_a.cost() > path_b.cost(); });

            const size_t mu = population_pool.size();

            /**
             * Compute exponential ranking probabilities
             */

            std::vector<double> probabilities_by_rank(mu);

            // c is the normalization factor
            auto c = T(0);

            for (int rank = 0; rank < mu; ++rank) {
                double probability = (1.0 - std::exp(-rank));
                probabilities_by_rank[rank] = probability;
                c += probability;
            }

            for (double& probability : probabilities_by_rank) {
                probability /= c;
            }

            /**
             * Select from population pool according to the ranking probabilities,
             * with replacement.
             */

            return sampling::sample_from_probabilities(
                population_pool, lambda, probabilities_by_rank.cbegin(),
                probabilities_by_rank.cend(), random_generator);
        }
    }  // namespace parent

    namespace children {
        // Perform the generational (μ, λ) selection. μ out of the λ offsprings are selected using
        // weighted sampling.
        template <typename T, class URBG>
        void generational_mu_lambda_selection(std::vector<PermutationPath<T>>& population_pool,
                                              std::vector<PermutationPath<T>>&& offspring_pool,
                                              URBG&& random_generator) noexcept {
            const size_t mu = population_pool.size();
            const size_t lambda = offspring_pool.size();

            /**
             * Compute probabilities of being removed from the offspring pool
             */

            std::vector<double> probabilities;
            probabilities.reserve(lambda);

            // c is the normalization factor
            auto c = T(0);

            for (PermutationPath<T>& permutation_path : offspring_pool) {
                double distance = permutation_path.cost();
                probabilities.push_back(distance);
                c += distance;
            }

            for (double& probability : probabilities) {
                probability /= c;
            }

            /**
             * Select from population pool according to the probabilities,
             * without replacement.
             */

            std::vector<size_t> selected_indexes(sampling::weighted_sample_indexes(
                probabilities.begin(), probabilities.end(), mu, random_generator));

            /**
             * Place the selected offsprings at the beginning of offspring_pool,
             * reduce the offspring_pool size from λ to μ and move it to population_pool.
             */

            /*
            std::sort(selected_indexes.begin(), selected_indexes.end());

            size_t i = 0;
            for (size_t j : selected_indexes) {
                // i <= j
                using std::swap;
                swap(offspring_pool[i], offspring_pool[j]);
                ++i;
            }

            utils::reduce_size(offspring_pool, mu);
            population_pool = std::move(offspring_pool);
            */

            std::vector<PermutationPath<T>> sample_result;
            sample_result.reserve(mu);

            for (size_t i : selected_indexes) {
                sample_result.emplace_back(offspring_pool[i]);
            }

            population_pool = std::move(sample_result);
        }
    }  // namespace children
}  // namespace selection