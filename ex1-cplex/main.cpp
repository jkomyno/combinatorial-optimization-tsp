#include <chrono>    // std::chrono
#include <iostream>  // std::cout, std::cerr
#include <optional>  // std::optional

#include <shared/DistanceMatrix.h>
#include <shared/read_tsp_file.h>
#include <shared/stopwatch.h>
#include <shared/path_utils/PermutationPath.h>

#include "cli.h"
#include "CPLEXModel.h"

int main(int argc, char** argv) {
    /**
     * Handler for command-line arguments
     */
    auto args = cli::parse(argc, argv);
    std::chrono::duration timeout_ms = std::chrono::milliseconds(args.timeout_ms);

    auto point_reader(read_tsp_file(args.filename.c_str()));

    const size_t N = point_reader->dimension;
    DistanceMatrix<double> distance_matrix = point_reader->create_distance_matrix();

    // start the stopwatch
    const auto program_time_start = stopwatch::now();

    CPLEXModel<double> cplex_model(distance_matrix, timeout_ms);

    cplex_model.solve();

    // stop the stopwatch
    auto program_time_stop = stopwatch::now();

    // executions of the program in ms
    const auto program_time_ms =
        stopwatch::duration<stopwatch::ms_t>(program_time_start, program_time_stop);

    const bool was_interrupted = program_time_ms - timeout_ms.count() > 0;

    std::cout << "N: " << N << '\n';
    std::cout << "program_time_ms: " << program_time_ms << '\n';
    std::cout << "was_interrupted: " << was_interrupted << '\n';

    std::optional<PermutationPath<double>> solution_path = cplex_model.get_solution();

    if (solution_path.has_value()) {
        std::cout << "solution_cost: " << std::fixed << solution_path->cost() << '\n';

        // Print path
        if (args.show_path) {
            std::cout << *solution_path << '\n';
        }

    } else {
        std::cout << "No solution found." << '\n';
    }

    std::cout << std::flush;
    return 0;
}
