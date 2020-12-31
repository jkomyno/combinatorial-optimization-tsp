#pragma once

#include "PermutationPath.h"

namespace mutation {
    // Perform the swap mutation on the given path.
    // Preconditions:
    // 1) 0 <= x <= N - 1
    // 2) 0 <= y <= N - 1
    template <typename T>
    inline void swap(PermutationPath<T>& path, size_t x, size_t y) noexcept {
        using std::swap;
        swap(path[x], path[y]);
    }

    // Perform the left-rotation mutation on the given path.
    // swap([0,1,2,3,4], 1, 3) => [0,2,3,1,4]
    // Preconditions:
    // 1) 0 <= x <= N - 1
    // 2) 0 <= y <= N - 1
    // 3) x < y
    template <typename T>
    inline void left_rotation(PermutationPath<T>& path, size_t x, size_t y) noexcept {
        size_t left = path[x];
        std::copy(path.begin() + x + 1, path.begin() + y + 1, path.begin() + x);
        path[y] = left;
    }

    // Perform the right-rotation mutation on the given path.
    // swap([0,1,2,3,4], 1, 3) => [0,3,1,2,4]
    // Preconditions:
    // 1) 0 <= x <= N - 1
    // 2) 0 <= y <= N - 1
    // 3) x < y
    template <typename T>
    inline void right_rotation(PermutationPath<T>& path, size_t x, size_t y) noexcept {
        size_t right = path[y];
        std::copy(path.begin() + x, path.begin() + y, path.begin() + x + 1);
        path[x] = right;
    }

    // Perform the inversion mutation on the given path, aka a 2-opt move.
    // inversion([0,1,2,3,4], 1, 3) => [0,4,3,2,1]
    // Preconditions:
    // 1) 0 <= x <= N - 1
    // 2) 0 <= y <= N - 1
    template <typename T>
    inline void inversion(PermutationPath<T>& path, size_t x, size_t y) noexcept {
        size_t i = x;
        size_t j = y;

        while (i < j) {
            using std::swap;
            swap(path[i], path[j]);
            ++i;
            --j;
        }
    }
}  // namespace mutation
