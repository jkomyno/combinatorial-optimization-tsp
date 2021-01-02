#pragma once

#include <algorithm>      // std::find_if
#include <random>         // std::mt19937, std::random_device
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

#include <shared/DistanceMatrix.h>

#include "MetaHeuristicsParams.h"
#include "PermutationPath.h"
#include "Solver.h"
#include "farthest_insertion.h"
#include "local_search.h"
#include "mating.h"
#include "mutation.h"
#include "population.h"
#include "sampling.h"
#include "selection.h"

template <typename T>
class TSPSolver : public Solver<PermutationPath<T>> {
    using super = Solver<PermutationPath<T>>;

    // Constant reference to the distance matrix of the TSP problem
    const DistanceMatrix<T>& distance_matrix;

    // Parameters that regulate the meta-heuristic algorithm search strategy
    MetaHeuristicsParams params;

    // Random generator instance
    std::mt19937 random_generator;

    // Size of the path, initialized at the beginning of ::solve()
    const size_t n = this->distance_matrix.size();

    // Compute the initial solution according to a heuristic.
    [[nodiscard]] PermutationPath<T> compute_initial_heuristic_solution() const noexcept {
        std::vector<size_t> circuit(heuristic::farthest_insertion(this->distance_matrix));
        return PermutationPath<T>(std::move(circuit), this->distance_matrix);
    }

    void mutate_with_probability(std::vector<PermutationPath<T>>& pool) noexcept {
        const double mutation_probability = this->params.mutation_probability;
        const auto should_mutate = [=](double probability) -> bool {
            return probability <= mutation_probability;
        };

        for (PermutationPath<T>& path : pool) {
            auto first = path.cbegin();
            auto last = path.cend();
            auto it = first + 1;

            // generate n - 1 random uniform probabilities in [0, 1)
            auto probabilities(sampling::sample_probabilities(it, last, this->random_generator));

            // select the indexes to mutate, i.e. the indexes where the probability is <= the
            // given mutation probability
            std::vector<size_t> indexes_to_mutate;

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

                mutation::left_rotation(path, x, y);
            }
        }
    }

    // Try to improve the current generation via local search
    void improve_generation(std::vector<T>& population_pool) noexcept {
        local_search::improve_generation_complete(population_pool);
    }

protected:
    // Return true when the first solution is better than the second.
    [[nodiscard]] bool solution_comparator(const PermutationPath<T>& a,
                                           const PermutationPath<T>& b) const noexcept override {
        return a.cost() < b.cost();
    }

    // Compute the initial population pool of size μ
    [[nodiscard]] std::vector<PermutationPath<T>>
    compute_initial_population_pool() noexcept override {
        PermutationPath<T> heuristic_solution(this->compute_initial_heuristic_solution());

        return population::generate_initial(std::move(heuristic_solution), this->params.mu, this->n,
                                            this->random_generator);
    }

    // Compute the mating pool of size λ of the current iteration.
    [[nodiscard]] std::vector<PermutationPath<T>> compute_current_mating_pool() noexcept override {
        const size_t k = 2;
        return selection::tournament(super::population_pool, this->params.lambda, k,
                                     this->random_generator);
    }

    // Compute the new generation of λ offsprings from a mating pool of size λ.
    [[nodiscard]] std::vector<PermutationPath<T>> compute_current_offspring_pool(
        std::vector<PermutationPath<T>>& mating_pool) noexcept override {
        return mating::sequential_crossover(mating_pool, this->n, this->random_generator);
    }

    // Perform a mutation of some of the given offsprings.
    void mutate_offsprings(std::vector<PermutationPath<T>>& offspring_pool) noexcept override {
        this->mutate_with_probability(offspring_pool);
    }

    // Select new generation's population pool.
    // TODO
    [[nodiscard]] std::vector<PermutationPath<T>> select_new_generation(
        std::vector<PermutationPath<T>>& mating_pool,
        std::vector<PermutationPath<T>>& offspring_pool) noexcept override {
        return mating_pool;
    }

    // Run a single iteration.
    void perform_iteration() noexcept {
        super::perform_iteration();
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

    // Run the solver
    void solve() noexcept override {
        // generate half of the population with random permutations of the initial heuristic path,
        // and the remaining half random permutations of the sorted path of cities [0,1,...,n-1]
        super::population_pool = this->compute_initial_population_pool();

        super::best_solution = {super::compute_current_best_solution()};

        while (super::n_generations_without_improvement <
                   this->params.max_n_generations_without_improvement &&
               super::n_generations < this->params.max_n_generations) {

            std::cout << "Generation #" << super::n_generations << std::endl;

            this->perform_iteration();
            this->update_best_solution();

            std::cout << "# generations without improvement: "
                      << this->n_generations_without_improvement << std::endl;
        }
    }
};
