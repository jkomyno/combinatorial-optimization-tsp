#include <chrono>    // std::chrono
#include <iostream>  // std::cerr, std::cout, std::endl
#include <string>    // std::stoi

#include <shared/DistanceMatrix.h>
#include <shared/path_utils/PermutationPath.h>
#include <shared/read_tsp_file.h>

#include "MetaHeuristicsParams.h"
#include "TSPSolver.h"

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

    std::cout << "Importing file" << '\n';
    auto point_reader(read_tsp_file(filename));

    const size_t N = point_reader->dimension;

    std::cout << "Creating distance matrix" << '\n';
    DistanceMatrix<double> distance_matrix = point_reader->create_distance_matrix();

    // clang-format off
	MetaHeuristicsParams params{
		// mutation_probability
		0.1,

		// crossover_rate
		0.8,

		// mu
		25,

		// lambda
		50,

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

    TSPSolver<double> tsp_solver(distance_matrix, params);
    tsp_solver.solve();
}
