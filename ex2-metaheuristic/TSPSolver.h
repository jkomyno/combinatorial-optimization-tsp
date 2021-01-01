#pragma once

#include <vector>  // std::vector

#include <shared/DistanceMatrix.h>

#include "MetaHeuristicsParams.h"
#include "PermutationPath.h"
#include "Solver.h"

template <typename T>
class TSPSolver : public Solver<PermutationPath<T>> {
    using super = Solver<PermutationPath<T>>;

    // Constant reference to the distance matrix of the TSP problem
    const DistanceMatrix<T>& distance_matrix;

    // Parameters that regulate the meta-heuristic algorithm search strategy
    MetaHeuristicsParams params;

protected:
    // Return true when the first solution is better than the second.
    [[nodiscard]] bool solution_comparator(const PermutationPath<T>& a,
                                           const PermutationPath<T>& b) const noexcept override {
        return a.cost() < b.cost();
    }

    // Compute the initial solution according to a heuristic.
    // TODO
    [[nodiscard]] PermutationPath<T> compute_initial_heuristic_solution() const noexcept override {
        return PermutationPath<T>({0, 1, 2, 3, 4}, this->distance_matrix);
    }

    // Compute the mating pool of size λ of the current iteration.
    // TODO
    [[nodiscard]] std::vector<PermutationPath<T>> compute_current_mating_pool() noexcept override {
        return {this->compute_initial_heuristic_solution()};
    }

    // Compute the new generation of λ offsprings from a mating pool of size λ.
    // TODO
    [[nodiscard]] std::vector<PermutationPath<T>> compute_current_offspring_pool(
        std::vector<PermutationPath<T>>& mating_pool) noexcept override {
        return mating_pool;
    }

    // Perform a mutation of some of the given offsprings.
    // TODO
    void mutate_offsprings(std::vector<PermutationPath<T>>& offspring_pool) noexcept override {
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
        params(params) {
    }

    ~TSPSolver() {
    }

    // Run the solver
    void solve() noexcept override {
        // generate half of the population with random permutations of the initial heuristic path,
        // and the remaining half random permutations of the sorted path of cities [0,1,...,n-1]
        super::population_pool = {this->compute_initial_heuristic_solution()};

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
