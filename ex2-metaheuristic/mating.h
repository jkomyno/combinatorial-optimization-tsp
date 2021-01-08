#pragma once

#include <random>  // std::uniform_real_distribution
#include <vector>  // std::vector

#include "PermutationPath.h"
#include "crossover.h"
#include "sampling.h"

namespace mating {
    // Select any two consecutive pairs from the mating pool and apply the crossover operation.
    // The new population pool will have the same size as the mating pool.
    // n is the size of each path.
    // The size of the mating pool must be even.
    template <typename T, class URBG>
    std::vector<PermutationPath<T>> sequential_crossover(
        std::vector<PermutationPath<T>>& mating_pool, size_t n, double crossover_rate, URBG&& random_generator) {

        const size_t lambda = mating_pool.size();
        std::vector<PermutationPath<T>> new_population_pool;
        new_population_pool.reserve(lambda);

		using distr_t = std::uniform_real_distribution<double>;
        using param_t = typename distr_t::param_type;
        distr_t distribution;

        for (size_t i = 0; i < lambda - 1; i += 2) {
            PermutationPath<T>& parent_1 = mating_pool[i];
            PermutationPath<T>& parent_2 = mating_pool[i + 1];

			const bool should_recombine = distribution(random_generator, param_t(0, 1)) >= crossover_rate;
			if (should_recombine) {
				auto&& [offspring_1, offspring_2] = crossover::order_alt(parent_1,
					parent_2, n, random_generator);

				utils::shift_to_value(offspring_1.begin(), offspring_1.end(), 0);
				utils::shift_to_value(offspring_2.begin(), offspring_2.end(), 0);

				new_population_pool.emplace_back(std::move(offspring_1));
				new_population_pool.emplace_back(std::move(offspring_2));
			} else {
				new_population_pool.emplace_back(parent_1);
				new_population_pool.emplace_back(parent_2);
			}
        }

        return new_population_pool;
    }

	// Select pairs at random from the mating pool and apply the crossover operation.
	// The new population pool will have the same size as the mating pool.
	// n is the size of each path.
	template <typename T, class URBG>
	std::vector<PermutationPath<T>> random_crossover(
		std::vector<PermutationPath<T>>& mating_pool, size_t n, double crossover_rate, URBG&& random_generator) {

        const size_t lambda = mating_pool.size();
        std::vector<PermutationPath<T>> new_population_pool;
        new_population_pool.reserve(lambda);

		using distr_t = std::uniform_real_distribution<double>;
        using param_t = typename distr_t::param_type;
        distr_t distribution;

		while (new_population_pool.size() < lambda) {
			auto&& selected_parent_indexes = sampling::sample_pair<false>(0, n, random_generator);

			PermutationPath<T>& parent_1 = mating_pool[selected_parent_indexes.first];
			PermutationPath<T>& parent_2 = mating_pool[selected_parent_indexes.second];

			const bool should_recombine = distribution(random_generator, param_t(0, 1)) >= crossover_rate;
			if (should_recombine) {
				auto&& [offspring_1, offspring_2] = crossover::order(parent_1,
					parent_2, n, random_generator);

				utils::shift_to_value(offspring_1.begin(), offspring_1.end(), 0);
				utils::shift_to_value(offspring_2.begin(), offspring_2.end(), 0);

				new_population_pool.emplace_back(std::move(offspring_1));
				new_population_pool.emplace_back(std::move(offspring_2));
			} else {
				new_population_pool.emplace_back(parent_1);
				new_population_pool.emplace_back(parent_2);
			}
		}

		return new_population_pool;
	}
}  // namespace mating