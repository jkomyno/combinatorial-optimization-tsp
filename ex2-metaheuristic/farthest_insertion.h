#pragma once

#include <vector> // std::vector
#include <unordered_set> // std::unordered_set

#include <shared/DistanceMatrix.h>

#include "utils.h"

namespace heuristic {
	template <typename T>
    std::vector<size_t> farthest_insertion(const DistanceMatrix<T>& distance_matrix) {
		const size_t size = distance_matrix.size();

        const auto get_distance = [&distance_matrix](const size_t x, const size_t y) -> T {
            return distance_matrix.at(x, y);
        };

		// Keep track of the nodes not in the partial Hamiltonian circuit.
		// Initially, none of them is in the circuit, so not_visited is initialized with
		// every vertex from 0 to size-1
		std::unordered_set<size_t> not_visited = utils::unordered_set_in_range(0, size);

		// Select the 2 farthest nodes and add them to the partial circuit
		const auto& [i, j] = distance_matrix.get_2_farthest_vertexes();

		// Keep track of the nodes in the partial Hamiltonian circuit
		std::vector<size_t> circuit{ i, j };
		circuit.reserve(size);

		// Remove the first 2 selected nodes from not_visited
		not_visited.erase(i);
		not_visited.erase(j);

		// Step 2: find a node k not in circuit that maximizes δ(k, circuit)
		const size_t r = utils::select_new_r_maximize(not_visited, circuit, get_distance);

		// Step 3: insert r in between the two consecutive tour cities i, j for which such an insertion
		// causes the minimum increase in total tour length.
		circuit.emplace_back(r);
		not_visited.erase(r);

		// Step 4: repeat the insertion from step 2 until all nodes have been inserted into the circuit
		while (!not_visited.empty()) {
			// Select the not visited node r that maximizes δ(r, circuit)
			size_t new_r = utils::select_new_r_maximize(not_visited, circuit, get_distance);
			not_visited.erase(new_r);

			// Find the arc (i, j) that minimizes the value of w(i, r) - w(r, j) - w(i, j)
			// and add r in between i and j in circuit
			utils::perform_best_circuit_insertion(new_r, circuit, get_distance);
		}

		// Force the solution to start from the first city, 0
		utils::shift_to_value<size_t>(circuit, 0);

		return circuit;
	}
}