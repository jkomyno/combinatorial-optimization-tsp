#include <chrono>    // std::chrono
#include <iostream>  // std::cout, std::cerr
#include <optional>  // std::optional

#include <shared/DistanceMatrix.h>
#include <shared/read_tsp_file.h>
#include <shared/stopwatch.h>

#include "RandomSolver.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "2 argument required: timeout_ms, filename" << std::endl;
        exit(1);
    }

    /**
     * read arguments
     */
    std::chrono::duration timeout_ms = std::chrono::milliseconds(std::stoi(argv[1]));
    const char* filename = argv[2];

    auto point_reader(read_tsp_file(filename));

    const size_t N = point_reader->dimension;
    DistanceMatrix<double> distance_matrix = point_reader->create_distance_matrix();

    // start the stopwatch
    const auto program_time_start = stopwatch::now();

    RandomSolver<double> random_solver(distance_matrix, timeout_ms);

    random_solver.solve();

    // stop the stopwatch
    auto program_time_stop = stopwatch::now();

    // executions of the program in ms
    const auto program_time_ms =
        stopwatch::duration<stopwatch::ms_t>(program_time_start, program_time_stop);

    const bool was_interrupted = program_time_ms - timeout_ms.count() > 0;

    std::cout << "N: " << N << '\n';
    std::cout << "program_time_ms: " << program_time_ms << '\n';
    std::cout << "was_interrupted: " << true << '\n';

    PermutationPath<double> solution_path(random_solver.get_solution());

    std::cout << "solution_cost: " << std::fixed << solution_path.cost();
    std::cout << std::flush;
}
