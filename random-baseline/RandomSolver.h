#pragma once

#include <algorithm>  // std::shuffle
#include <atomic>     // std::atomic
#include <chrono>     // std::chrono::duration
#include <future>     // std::future
#include <random>     // std::mt19937, std::random_device
#include <thread>     // std::thread, std::this_thread

#include <shared/DistanceMatrix.h>
#include <shared/path_utils/PermutationPath.h>

template <typename T>
class RandomSolver {
    // Constant reference to the distance matrix of the TSP problem
    const DistanceMatrix<T>& distance_matrix;

    // Keep track of the best solution
    PermutationPath<T> best_solution;

    // Maximum timeout for the random solver
    std::chrono::milliseconds timeout_ms;

    // Random generator instance
    std::mt19937 random_generator;

public:
    explicit RandomSolver(const DistanceMatrix<T>& distance_matrix,
                          std::chrono::milliseconds timeout_ms) noexcept :
        distance_matrix(distance_matrix),
        best_solution(PermutationPath<T>::from_size(distance_matrix.size(), distance_matrix)),
        timeout_ms(timeout_ms),
        random_generator(std::random_device()()) {
    }

    ~RandomSolver() {
    }

    // Perform a time-limited random search of the best solution
    void solve() noexcept {
        std::atomic<bool> is_time_expired = false;

        std::thread find_best_thread([this, &is_time_expired]() {
            // start from a copy of [0, 1, ..., n-1]
            PermutationPath<T> current_solution(this->best_solution);

            while (!is_time_expired) {
                // shuffle the current solution until it becomes better than the previous best
                // solution. Repeat until the time is expired.
                std::shuffle(current_solution.begin(), current_solution.end(),
                             this->random_generator);
                if (current_solution.cost() < this->best_solution.cost()) {
                    this->best_solution = current_solution;
                }
            }
        });

        // block the invoking thread for timeout_ms
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));

        // signal to find_best_thread that the time is up
        is_time_expired = true;
        find_best_thread.join();
    }

    // Return the best solution found by the random search
    PermutationPath<T> get_solution() noexcept {
        return this->best_solution;
    }
};
