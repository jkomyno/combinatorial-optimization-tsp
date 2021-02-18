#include <shared/DistanceMatrix.h>
#include <shared/path_utils/PermutationPath.h>
#include <shared/read_tsp_file.h>
#include <shared/stopwatch.h>

#include <chrono>  // std::chrono
#include <condition_variable>
#include <iostream>  // std::cerr, std::cout, std::endl
#include <mutex>
#include <string>  // std::stoi
#include <thread>

#include "MetaHeuristicsParams.h"
#include "TSPSolver.h"
#include "cli.h"

int main(int argc, char** argv) {
    /**
     * Handler for command-line arguments
     */
    auto args = cli::parse(argc, argv);
    std::chrono::duration timeout_ms = std::chrono::milliseconds(args.timeout_ms);

    // clang-format off
	MetaHeuristicsParams params{
		// mutation_probability
		args.mutation_probability,

		// crossover_rate
		args.crossover_rate,

		// mu
		args.mu,

		// lambda
		args.lambda,

		// max_n_generations_without_improvement
		args.max_gen_no_improvement,

		// max_n_generations
		args.max_gen,

		// tournament_k
		args.k,

		// restarts
		1
	};
    // clang-format on

    auto point_reader(read_tsp_file(args.filename.c_str()));

    const size_t N = point_reader->dimension;
    DistanceMatrix<double> distance_matrix = point_reader->create_distance_matrix();

    // start the stopwatch
    const auto program_time_start = stopwatch::now();

    TSPSolver<double> tsp_solver(distance_matrix, params);

    std::mutex m;
    std::condition_variable cv;
    bool was_interrupted = false;

    // spawns a new thread for solving the TSP problem
    std::thread thread_solver([&]() {
        tsp_solver.solve();
        std::lock_guard<std::mutex> lock(m);
        was_interrupted = tsp_solver.is_stopped();
        cv.notify_one();
    });

    {
        // blocks the main thread for timeout_ms milliseconds, or until the solver
        // finishes its task, whatever comes first.
        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, timeout_ms);
        tsp_solver.stop();
    }

    thread_solver.join();

    // stop the stopwatch
    auto program_time_stop = stopwatch::now();

    // executions of the program in ms
    const auto program_time_ms =
        stopwatch::duration<stopwatch::ms_t>(program_time_start, program_time_stop);

    std::cout << "N: " << N << '\n';
    std::cout << "program_time_ms: " << program_time_ms << '\n';
    std::cout << "was_interrupted: " << was_interrupted << '\n';
    std::cout << "Solution cost: " << std::fixed << (tsp_solver.get_best_solution())->cost()
              << '\n';

    if (args.show_path) {
        std::cout << "Solution path:\n" << *(tsp_solver.get_best_solution()) << '\n';
    }
}
