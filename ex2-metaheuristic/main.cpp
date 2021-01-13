#include <chrono>    // std::chrono
#include <iostream>  // std::cerr, std::cout, std::endl
#include <string>    // std::stoi

#include <argagg/argargg.h>

#include <shared/DistanceMatrix.h>
#include <shared/path_utils/PermutationPath.h>
#include <shared/read_tsp_file.h>

#include "MetaHeuristicsParams.h"
#include "TSPSolver.h"

int main(int argc, char** argv) {
    /**
     * Command-line parsing starts
     */

    argagg::parser argparser{
        {{"help", {"-h", "--help"}, "displays help message", 0},
         {"timeout_ms", {"-t", "--timeout-ms"}, "Timeout in milliseconds", 1},
         {"filename", {"-f", "--filename"}, "TSP dataset filename", 1},
         {"mutation_probability", {"-m", "--mutation-probability"}, "GA mutation probability", 1},
         {"crossover_rate", {"-c", "--crossover-rate"}, "GA crossover rate", 1},
         {"mu", {"--mu"}, "GA population pool size", 1},
         {"lambda", {"--lambda"}, "GA offspring pool size", 1}}};

    std::ostringstream usage;
    usage << "Metaheuristic solver\n"
          << '\n'
          << "Usage: " << argv[0] << " [options] ARG [ARG...]\n"
          << '\n';

    // Use our argument parser to... parse the command line arguments. If there
    // are any problems then just spit out the usage and help text and exit.
    argagg::parser_results args;
    try {
        args = argparser.parse(argc, argv);
    } catch (const std::exception& e) {
        argagg::fmt_ostream help(std::cerr);
        help << usage.str() << argparser << '\n'
             << "Encountered exception while parsing arguments: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    // If the help flag was specified then spit out the usage and help text and
    // exit.
    if (args["help"]) {
        argagg::fmt_ostream help(std::cerr);
        help << usage.str() << argparser;
        return EXIT_SUCCESS;
    }

    std::chrono::duration timeout_ms = std::chrono::milliseconds(args["timeout_ms"].as<int>());
    const char* filename = args["filename"].as<std::string>().c_str();

    // clang-format off
	auto mutation_probability = args["mutation_probability"].as<double>(0.010890687);
    auto crossover_rate       = args["crossover_rate"].as<double>(0.8305666658615982);
    auto mu                   = args["mu"].as<size_t>(35);
    auto lambda               = args["lambda"].as<size_t>(53);
    // clang-format on

    /**
     * Command-line parsing ends
     */

    // clang-format off
	MetaHeuristicsParams params{
		// mutation_probability
		mutation_probability,

		// crossover_rate
		crossover_rate,

		// mu
		mu,

		// lambda
		lambda,

		// max_n_generations_without_improvement
		200,

		// max_n_generations
		500,

		// tournament_k
		12,

		// restarts
		1
	};
    // clang-format on

    auto point_reader(read_tsp_file(filename));

    const size_t N = point_reader->dimension;
    std::cout << "N: " << N << '\n';

    // std::cout << "Creating distance matrix" << '\n';
    DistanceMatrix<double> distance_matrix = point_reader->create_distance_matrix();

    TSPSolver<double> tsp_solver(distance_matrix, params);
    tsp_solver.solve();

    std::cout << "Solution cost: " << std::fixed << tsp_solver.get_best_solution().value().cost()
              << '\n';
}
