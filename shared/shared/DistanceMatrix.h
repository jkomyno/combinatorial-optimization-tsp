#pragma once

#include <algorithm>  // std::max_element
#include <cmath>      // std::sqrt
#include <iomanip>    // std::setw
#include <iostream>   // std::ostream
#include <numeric>    // std::iota
#include <vector>     // std::vector

#include "Matrix.h"

/**
 * DistanceMatrix represents a distance matrix for a complete, weighted, undirected graph.
 * It's a symmetric matrix which main diagonal is filled with 0s.
 * It stores its values in a contiguous 1D vector.
 */
template <typename T = double>
class DistanceMatrix : public Matrix<T> {
    size_t n_vertexes;

    // Initialize the distance matrix according to the distance(i, j) function.
    // The vector data is already filled with 0s.
    template <typename Distance>
    void init(const Distance& distance) noexcept {
        const size_t dimension = size();

        // Populate the triangle above the main diagonal
        for (size_t i = 0; i < dimension; ++i) {
            for (size_t j = i + 1; j < dimension; ++j) {
                this->at(i, j) = distance(i, j);
            }
        }

        // Symmetrically copies the triangle above the main diagonal to the triangle
        // below the main diagonal
        for (size_t i = dimension - 1; i > 0; --i) {
            for (size_t j = 0; j < i; ++j) {
                this->at(i, j) = this->at(j, i);
            }
        }
    }

public:
    // Create a new square matrix with n_vertexes rows initialized to all 0s.
    // distance(i, j) returns the distance between the i-th and j-th point.
    template <typename Distance>
    DistanceMatrix(size_t n_vertexes, Distance&& distance) noexcept :
        Matrix<T>(n_vertexes, n_vertexes, T(0)),
        n_vertexes(n_vertexes) {
        init(std::forward<Distance>(distance));
    }

    // Constructor left for test purposes
    DistanceMatrix(const std::vector<T>& vec, size_t n_vertexes) noexcept :
        Matrix<T>(n_vertexes, n_vertexes, vec),
        n_vertexes(n_vertexes) {
    }

    // Return number of rows/columns of the matrix
    [[nodiscard]] size_t size() const noexcept {
        return this->rows;
    }

    // Return the vertexes in the distance matrix
    [[nodiscard]] std::vector<size_t> get_vertexes() const noexcept {
        std::vector<size_t> v(n_vertexes);
        std::iota(v.begin(), v.end(), 0);
        return v;
    }

    // Return the two farthest vertexes in the matrix
    [[nodiscard]] std::pair<size_t, size_t> get_2_farthest_vertexes() const {
        const auto it_begin = this->upper_triangular_cbegin();
        const auto it_end = this->upper_triangular_cend();
        const auto it_max = std::max_element(it_begin, it_end);
        return it_max.get_row_column();
    }
};
