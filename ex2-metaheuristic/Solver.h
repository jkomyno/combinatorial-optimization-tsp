#pragma once

#include <optional>  // std::optional
#include <vector>    // std::vector

// Abstract genetic algorithm solver
template <typename T>
class Solver {
protected:
    // Store the population pool
    std::vector<T> population_pool;

    // Keep track of the best solution so far
    std::optional<T> best_solution;

    // Number of the generation that gave birth to the best solution
    size_t best_solution_index = 0;

    // Sequential number that keeps track of the number of generations
    size_t n_generations = 0;

    // Keep track of the number of generations without improvement
    size_t n_generations_without_improvement = 0;

    // Return true when the first solution is better than the second.
    [[nodiscard]] virtual bool solution_comparator(const T& a, const T& b) const noexcept = 0;

    // Compute the initial population pool
    [[nodiscard]] virtual std::vector<T> compute_initial_population_pool() noexcept = 0;

    // Compute the mating pool of size λ of the current iteration
    [[nodiscard]] virtual std::vector<T> compute_current_mating_pool() noexcept = 0;

    // Compute the new generation of λ offsprings from a mating pool of size λ.
    [[nodiscard]] virtual std::vector<T> compute_current_offspring_pool(
        std::vector<T>& mating_pool) noexcept = 0;

    // Perform a mutation of some of the given offsprings
    virtual void mutate_offsprings(std::vector<T>& offspring_pool) noexcept = 0;

    // Select new generation's population pool
    virtual void select_new_generation(std::vector<T>&& mating_pool,
                                       std::vector<T>&& offspring_pool) noexcept = 0;

    // If true, create a new generation iteration
    [[nodiscard]] virtual bool should_continue() noexcept = 0;

    // Run a single iteration
    virtual void perform_iteration() noexcept {
        // Increment the number of generations
        this->n_generations++;
    }

    // Compute the best solution of the given pool and returns a copy of it.
    // The pool must contain at least one element.
    [[nodiscard]] T compute_best_solution(std::vector<T>& pool) const noexcept {
        auto comparator = [this](const auto& a, const auto& b) -> bool {
            return this->solution_comparator(a, b);
        };

        auto best_it = std::min_element(pool.cbegin(), pool.cend(), comparator);

        return T(*best_it);
    }

    void update_best_solution() noexcept {
        const T current_best_solution = this->compute_best_solution(this->population_pool);

        if (this->solution_comparator(current_best_solution, this->best_solution.value())) {

            this->best_solution = {std::move(current_best_solution)};
            this->n_generations_without_improvement = 0;
        } else {
            this->n_generations_without_improvement++;
        }
    }

public:
    explicit Solver() noexcept {
    }

    virtual ~Solver() noexcept {
    }

    // Run the solver
    virtual void solve() noexcept = 0;

    // Return the best solution
    [[nodiscard]] std::optional<T> get_best_solution() const noexcept {
        return best_solution;
    }
};
