#pragma once

#include <iterator>  // std::next

namespace utils {
    // Compute the total distance of the circuit path [start, ..., end, start].
    template <class ForwardIt, typename Distance>
    auto sum_distances_as_circuit(const ForwardIt& start, const ForwardIt& end,
                                  Distance&& get_distance) noexcept {
        auto circuit_distance = 0;
        auto it_prev = start;

        for (auto it_curr = std::next(start, 1); it_curr != end; ++it_curr) {
            auto distance = get_distance(*it_curr, *it_prev);
            circuit_distance += distance;
            ++it_prev;
        }

        circuit_distance += get_distance(*start, *it_prev);
        return circuit_distance;
    }
}  // namespace utils