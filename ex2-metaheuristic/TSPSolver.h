﻿#pragma once

#include <shared/DistanceMatrix.h>
#include <shared/path_utils/PermutationPath.h>

#include <algorithm>      // std::find_if
#include <random>         // std::mt19937, std::random_device
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

#include "MetaHeuristicsParams.h"
#include "Solver.h"
#include "SolverTablePrinter.h"
#include "decay.h"
#include "farthest_insertion.h"
#include "local_search.h"
#include "mating.h"
#include "mutation.h"
#include "population.h"
#include "sampling.h"
#include "selection.h"
#include "statistics.h"

namespace solver {
    enum class SelectionType { TOURNAMENT, EXPONENTIAL_RANKING };
    enum class MutationType { SWAP, LEFT_ROTATION, RIGHT_ROTATION, INVERSION };
}  // namespace solver

template <typename T, bool elitism = true,
          solver::SelectionType selection_type = solver::SelectionType::TOURNAMENT,
          solver::MutationType mutation_type = solver::MutationType::LEFT_ROTATION>
class TSPSolver : public Solver<PermutationPath<T>> {
    using super = Solver<PermutationPath<T>>;

    // Constant reference to the distance matrix of the TSP problem
    const DistanceMatrix<T>& distance_matrix;

    // Parameters that regulate the meta-heuristic algorithm search strategy
    MetaHeuristicsParams params;

    // Random generator instance
    std::mt19937 random_generator;

    // Size of the path
    const size_t n = this->distance_matrix.size();

    // Heuristic solution
    PermutationPath<T> heuristic_solution =
        TSPSolver::compute_initial_heuristic_solution<T>(this->distance_matrix);

    // When stop is set to true, the solver should stop
    volatile bool stop_cond = false;

    // Compute the initial solution according to a heuristic.
    template <typename V>
    static PermutationPath<V> compute_initial_heuristic_solution(
        const DistanceMatrix<T>& distance_matrix) noexcept {
        std::vector<size_t> circuit(heuristic::farthest_insertion(distance_matrix));
        PermutationPath<V> heuristic_path(std::move(circuit), distance_matrix);

        // initialize cost
        heuristic_path.cost();

        return heuristic_path;
    }

    void mutate_with_probability(std::vector<PermutationPath<T>>& pool) noexcept {
        const double mutation_probability = this->params.mutation_probability;
        const auto should_mutate = [=](double probability) -> bool {
            return probability <= mutation_probability;
        };

        for (PermutationPath<T>& path : pool) {
            // generate n - 1 random uniform probabilities in [0, 1)
            auto probabilities(sampling::sample_probabilities(std::next(path.cbegin()), path.cend(),
                                                              this->random_generator));

            // select the indexes to mutate, i.e. the indexes where the probability is <= the
            // given mutation probability
            std::vector<size_t> indexes_to_mutate;

            auto first = probabilities.cbegin();
            auto last = probabilities.cend();
            auto it = std::next(first);

            while ((it = std::find_if(it, last, should_mutate)) != last) {
                indexes_to_mutate.push_back(std::distance(first, it));
                ++it;
            }

            // if the selected indexes are odd, drop the last one
            if (indexes_to_mutate.size() % 2) {
                indexes_to_mutate.pop_back();
            }

            // apply the mutation for every sequential pair of selected indexes
            for (size_t i = 0; i < indexes_to_mutate.size(); i += 2) {
                size_t x = indexes_to_mutate[i];
                size_t y = indexes_to_mutate[i + 1];

                using namespace solver;

                if constexpr (mutation_type == MutationType::SWAP) {
                    mutation::swap(path, x, y);
                } else if constexpr (mutation_type == MutationType::LEFT_ROTATION) {
                    mutation::left_rotation(path, x, y);
                } else if constexpr (mutation_type == MutationType::RIGHT_ROTATION) {
                    mutation::right_rotation(path, x, y);
                } else if constexpr (mutation_type == MutationType::INVERSION) {
                    mutation::inversion(path, x, y);
                }
            }
        }
    }

    // Try to improve the initial generation via local search
    void improve_initial_generation(std::vector<PermutationPath<T>>& population_pool) noexcept {
        local_search::improve_generation_simple(population_pool, this->random_generator);
    }

    // Try to improve the current generation via local search
    void improve_generation(std::vector<PermutationPath<T>>& population_pool) noexcept {
        local_search::improve_generation_greedy(population_pool, this->random_generator);
    }

protected:
    // Return true when the first solution is better than the second.
    [[nodiscard]] bool solution_comparator(const PermutationPath<T>& a,
                                           const PermutationPath<T>& b) const noexcept override {
        return a.cost() < b.cost();
    }

    // Compute the initial population pool of size μ
    [[nodiscard]] std::vector<PermutationPath<T>> compute_initial_population_pool() noexcept
        override {
        return population::generate_initial(this->heuristic_solution, this->params.mu,
                                            this->random_generator);
    }

    // Compute the mating pool of size λ of the current iteration.
    [[nodiscard]] std::vector<PermutationPath<T>> compute_current_mating_pool() noexcept override {
        using namespace selection;
        using namespace solver;

        if constexpr (selection_type == SelectionType::EXPONENTIAL_RANKING) {
            return parent::ranking(super::population_pool, this->params.lambda,
                                   this->random_generator);
        } else if constexpr (selection_type == SelectionType::TOURNAMENT) {
            return parent::tournament(super::population_pool, this->params.lambda,
                                      this->params.tournament_k, this->random_generator);
        }
    }

    // Compute the new generation of λ offsprings from a mating pool of size λ.
    [[nodiscard]] std::vector<PermutationPath<T>> compute_current_offspring_pool(
        std::vector<PermutationPath<T>>& mating_pool) noexcept override {
        return mating::sequential_crossover(mating_pool, this->n, this->params.crossover_rate,
                                            this->random_generator);
    }

    // Perform a mutation of some of the given offsprings.
    void mutate_offsprings(std::vector<PermutationPath<T>>& offspring_pool) noexcept override {
        this->mutate_with_probability(offspring_pool);
    }

    // Select new generation's population pool.
    // It implements a (μ, λ) selection with elitism.
    void select_new_generation(std::vector<PermutationPath<T>>&& mating_pool,
                               std::vector<PermutationPath<T>>&& offspring_pool) noexcept override {

        using namespace selection;

        PermutationPath<T> best_father(super::compute_best_solution(mating_pool));

        children::generational_mu_lambda_selection(
            super::population_pool, std::move(offspring_pool), this->random_generator);

        if constexpr (elitism) {
            const T best_selected_offspring_cost =
                super::compute_best_solution(super::population_pool).cost();

            if (best_selected_offspring_cost > best_father.cost()) {
                using std::swap;
                swap(super::population_pool[1], best_father);
            }
        }
    }

    // Return true if and only if the genetic algorithm should continue, i.e. if the number of
    // generations without improvement and the number of generations haven't reached the max
    bool should_continue() noexcept override {
        if (this->stop_cond) {
            return false;
        }

        return super::n_generations_without_improvement <
                   this->params.max_n_generations_without_improvement &&
               super::n_generations < this->params.max_n_generations;
    }

    // Perform a single iteration of the genetic algorithm
    void perform_iteration() noexcept override {
        // Select λ members of a pool of μ individuals to create a mating pool.
        std::vector<PermutationPath<T>> mating_pool(this->compute_current_mating_pool());

        // Create the a new generation of λ offsprings from a mating pool of size λ.
        std::vector<PermutationPath<T>> offspring_pool(
            this->compute_current_offspring_pool(mating_pool));

        // Perform a mutation of some offsprings
        this->mutate_offsprings(offspring_pool);

        // Select new generation's population pool
        this->select_new_generation(std::move(mating_pool), std::move(offspring_pool));

        super::perform_iteration();
    }

    void init() noexcept {
        super::population_pool = this->compute_initial_population_pool();

        // Compute the best solution
        super::best_solution = {super::compute_best_solution(super::population_pool)};

        std::cout << "Heuristic cost: " << this->heuristic_solution.cost() << '\n';
        std::cout << "Best cost: " << super::best_solution.value().cost() << '\n';

        this->improve_initial_generation(super::population_pool);
        super::best_solution = {super::compute_best_solution(super::population_pool)};

        std::cout << "Improved cost: " << super::best_solution.value().cost() << '\n';
    }

public:
    explicit TSPSolver(const DistanceMatrix<T>& distance_matrix,
                       const MetaHeuristicsParams& params) noexcept :
        super(),
        distance_matrix(distance_matrix),
        params(params),
        random_generator(std::random_device()()) {
    }

    ~TSPSolver() {
    }

    void stop() noexcept {
        this->stop_cond = true;
    }

    bool is_stopped() const noexcept {
        return this->stop_cond;
    }

    // Run the solver
    void solve() noexcept override {
        // initialize first solutions
        this->init();

        // verbose output
        solver::table::header(std::cout);

        while (this->should_continue()) {
            const double avg_cost = statistics::average_cost(super::population_pool);
            const double previous_best_cost = super::best_solution.value().cost();

            this->perform_iteration();

            if (super::n_generations_without_improvement % 20) {
                this->improve_generation(super::population_pool);
            }

            this->update_best_solution();

            const double current_best_cost = super::best_solution.value().cost();

            solver::table::row(std::cout, super::n_generations, previous_best_cost,
                               current_best_cost, avg_cost,
                               super::n_generations_without_improvement);
        }

        auto&& current_best_solution = super::compute_best_solution(super::population_pool);

        std::cout << '\n' << "Best of generations: " << current_best_solution.cost() << '\n';
        this->improve_generation(super::population_pool);

        this->update_best_solution();
    }
};
