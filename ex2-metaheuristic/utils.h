#pragma once

#include <algorithm>      // std::generate_n, std::max_element, std::find, std::rotate
#include <iterator>       // std::next, std::inserter
#include <limits>         // std::numeric_limits
#include <list>           // std::list
#include <type_traits>    // std::forward
#include <unordered_map>  // std::unordered_map
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

namespace utils {
    // Compute the total distance of the circuit path [start, ..., end, start].
    template <class ForwardIt, typename Distance>
    auto sum_distances_as_circuit(const ForwardIt& start, const ForwardIt& end,
                                  Distance&& get_distance) noexcept {
        using distance_t = decltype(get_distance(0, 0));
        auto circuit_distance = distance_t(0);
        auto it_prev = start;

        for (auto it_curr = std::next(start, 1); it_curr != end; ++it_curr) {
            auto distance = get_distance(*it_curr, *it_prev);
            circuit_distance += distance;
            ++it_prev;
        }

        circuit_distance += get_distance(*start, *it_prev);
        return circuit_distance;
    }

    // Find the arc (i, j) that minimizes the value of w(i, r) - w(r, j) - w(i, j)
    // and add k in between i and j in circuit.
    template <typename Distance>
    void perform_best_circuit_insertion(const size_t r, std::vector<size_t>& circuit,
                                        Distance&& get_distance) {
        const size_t size = circuit.size();

        // Circuit_combinations will be sequentially modified to represent all possible insertions
        // of the vertex r between the vertexes in circuit. We prefer this mutable approach rather
        // than first enumerating all possible insertions and then selecting the best to keep the
        // space complexity at O(n) instead of O(n^2).
        // Initially, it is an ordered copy of the given circuit.
        // We use std::list because insertions and deletions at any point are O(1).
        std::list<size_t> circuit_insertion_list(circuit.cbegin(), circuit.cend());

        // Compute the first insertion possibility's weight.
        // For example, suppose that circuit is {3,4,1} and that r = 0.
        // The possible insertions are {0,3,4,1}, {3,0,4,1}, and {3,4,0,1}.
        // The following block sets circuit_insertion_list to the first possible insertion
        // ({0,3,4,1}, according to the example) and saves its circuit weight, i.e.
        // w(0,3) + w(3,4) + w(4,1) + w(1,0).
        auto it_list = circuit_insertion_list.begin();
        it_list = circuit_insertion_list.insert(it_list, r);
        const auto first_weight = sum_distances_as_circuit(
            circuit_insertion_list.cbegin(), circuit_insertion_list.cend(), get_distance);

        // keep track of the minimum weight
        auto min_weight = first_weight;

        // Keep track of the index of the list where the circuit weight is minimized
        size_t index_min_weight = 0;

        // Compute the other possible insertions removing r from circuit_insertion_list and adding
        // it again in the next suitable position. Removals and insertions are O(1) because we're
        // using lists. Since the total possible insertions in a list with n elements is n and we
        // already computed the first possible insertion, we need only n-2 other iterations.
        for (size_t i = 1; i < size; ++i) {
            it_list = circuit_insertion_list.erase(it_list);
            it_list = circuit_insertion_list.insert(std::next(it_list), r);
            const auto weight = sum_distances_as_circuit(
                circuit_insertion_list.cbegin(), circuit_insertion_list.cend(), get_distance);

            if (weight < min_weight) {
                min_weight = weight;
                index_min_weight = i;
            }
        }

        // Add r to the place where the circuit weight is minimized.
        // This operation takes O(n), because circuit is a vector.
        const auto it_insert = std::next(circuit.begin(), index_min_weight);
        circuit.insert(it_insert, r);
    }

    // Generate an unordered set with integer values in range [low, high)
    [[nodiscard]] inline std::unordered_set<size_t> unordered_set_in_range(size_t low,
                                                                           size_t high) noexcept {
        const size_t size = high - low;
        std::unordered_set<size_t> set;
        set.reserve(size);

        std::generate_n(std::inserter(set, set.end()), size,
                        [&set, low] { return set.size() + low; });

        return set;
    }

    // Return the maximum element in a list of elements.
    // This std::max_element wrapper is necessary to force the compiler to choose the right overload
    // of a higher-order function at compile-time.
    // See: https://stackoverflow.com/a/36794145/6174476
    const auto max_element = [](auto&&... args) -> decltype(auto) {
        return std::max_element(std::forward<decltype(args)>(args)...);
    };

    // Return the vertex r that doesn't belong to the partial Hamiltonian circuit that
    // maximizes the distance δ(r, circuit).
    // get_distance is the distance function that computes the cost between 2 nodes.
    template <typename Distance>
    [[nodiscard]] size_t select_new_r_maximize(std::unordered_set<size_t>& not_visited,
                                               std::vector<size_t>& circuit,
                                               Distance&& get_distance) noexcept {
        return select_new_r(not_visited, circuit, get_distance, max_element);
    }

    // Return the vertex r that doesn't belong to the partial Hamiltonian circuit that
    // maximizes or minimizes the distance δ(r, circuit) w.r.t. get_best_r.
    // get_distance is the distance function that computes the cost between 2 nodes.
    template <class GetBestR, typename Distance>
    [[nodiscard]] size_t select_new_r(std::unordered_set<size_t>& not_visited,
                                      std::vector<size_t>& circuit, Distance&& get_distance,
                                      GetBestR&& get_best_r) noexcept {
        using distance_t = decltype(get_distance(0, 0));

        // map that stores the minimum distance for each candidate vertex k
        std::unordered_map<size_t, distance_t> node_min_weight_map;
        node_min_weight_map.reserve(not_visited.size());

        for (const auto r : not_visited) {
            distance_t min_hk_weight = std::numeric_limits<distance_t>::max();
            for (const auto h : circuit) {
                const distance_t weight = get_distance(h, r);

                if (weight < min_hk_weight) {
                    min_hk_weight = weight;
                }
            }

            // update the map with the minimum weight between h and r found up to now
            node_min_weight_map[r] = min_hk_weight;
        }

        // both std::min_element and std::max_element need a comparator that returns true when x < y
        auto map_comparator = [](const auto& x, const auto& y) { return x.second < y.second; };

        // maximize or minimize distances based on comparator
        const auto it_new_r = get_best_r(node_min_weight_map.cbegin(), node_min_weight_map.cend(),
                                         std::move(map_comparator));

        // obtain the maximum of the maximum or minimum distances δ(r, circuit)
        const size_t new_r = it_new_r->first;
        return new_r;
    }

    // Shift a vector without removing items such that the given value is placed in first position.
    // We assume that all elements in the vector are different and that n exists in the given
    // vector.
    template <typename T>
    void inline shift_to_value(std::vector<T>& vec, const T& value) noexcept {
        auto n_it = std::find(vec.begin(), vec.end(), value);
        std::rotate(vec.begin(), n_it, vec.end());
    }
}  // namespace utils