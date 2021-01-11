#pragma once

#include <iterator>  // std::next
#include <numeric>   // std::iota
#include <vector>    // std::vector

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

    // Generate a vector with integer values in range [low, high)
    [[nodiscard]] inline std::vector<size_t> vector_in_range(size_t low, size_t high) noexcept {
        std::vector<size_t> vec(high - low);
        std::iota(vec.begin(), vec.end(), low);

        return vec;
    }
}  // namespace utils