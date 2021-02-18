#pragma once

#include <iomanip>   // std::setw
#include <iostream>  // std::ostream
#include <iterator>  // std::forward_iterator_tag
#include <vector>    // std::vector

/**
 * 2D matrix where values are stored in a contiguous 1D vector.
 */
template <typename T = double>
class Matrix {
protected:
    const size_t rows;
    const size_t cols;
    std::vector<T> data;

    using ForwardIt = decltype(data.cbegin());

    // Maps a matrix index pair to a vector index
    [[nodiscard]] size_t get_index(size_t row, size_t column) const noexcept {
        return row * cols + column;
    }

public:
    explicit Matrix(size_t rows, size_t cols, T default_value = T(0)) noexcept :
        rows(rows), cols(cols), data(rows * cols, default_value) {
    }

    explicit Matrix(size_t rows, size_t cols, const std::vector<T>& vec) noexcept :
        rows(rows), cols(cols), data(vec) {
    }

    // iterator for the upper triangle of the matrix, useful for computing the maximum/minimum value
    // in O(n/2)
    template <typename V>
    struct upper_triangular_iterator {
        using iterator_t = upper_triangular_iterator<V>;
        using value_type = V;
        using pointer = V*;
        using reference = V&;
        using const_reference = const V&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        explicit upper_triangular_iterator(pointer p, size_t N, size_t row, size_t column) :
            p(p), N(N), row(row), column(column) {
        }

        // Copy-assignment operator
        iterator_t& operator=(const iterator_t& other) {
            if (this != &other) {
                p = other.p;
                N = other.N;
                row = other.row;
                column = other.column;
            }

            return *this;
        }

        reference operator*() {
            return *std::next(p, row * N + column);
        }

        const_reference operator*() const {
            return *std::next(p, row * N + column);
        }

        iterator_t& operator++() {
            if (column == N - 1) {
                ++row;
                column = row + 1;
            } else {
                ++column;
            }

            return *this;
        }

        // Equality boolean operator
        friend bool operator==(const iterator_t& lhs, const iterator_t& rhs) {
            return lhs.row == rhs.row && lhs.column == rhs.column;
        }

        // Inequality boolean operator
        friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs) {
            return !(lhs == rhs);
        }

        // Retrieve the (row, column) pair at the current iterator position
        std::pair<size_t, size_t> get_row_column() const {
            return {row, column};
        }

    private:
        pointer p;
        size_t N;
        size_t row;
        size_t column;
    };

    // Return the raw T* distance matrix
    [[nodiscard]] const T* raw_data() const noexcept {
        return data.data();
    }

    // Retrieve the value saved at position (i, j)
    [[nodiscard]] const T& at(size_t i, size_t j) const noexcept {
        return data.at(get_index(i, j));
    }

    // Set the value for position (i, j)
    [[nodiscard]] T& at(size_t i, size_t j) noexcept {
        return data.at(get_index(i, j));
    }

    // Initial iterator for the upper triangle of the matrix
    auto upper_triangular_cbegin() const {
        const auto N = rows;
        return upper_triangular_iterator<T>(const_cast<T*>(data.data()), N, 0, 1);
    }

    // Final iterator for the upper triangle of the matrix
    auto upper_triangular_cend() const {
        const auto N = rows;
        return upper_triangular_iterator<T>(const_cast<T*>(data.data()), N, N - 1, N);
    }

    // Pretty-print distance matrix, useful for debugging/visualization purposes
    friend std::ostream& operator<<(std::ostream& os, const Matrix<T>& matrix) {
        const auto width = 8;
        const size_t size = matrix.rows;

        for (size_t row = 0; row < size - 1; ++row) {
            for (size_t column = 0; column < size - 1; ++column) {
                os << std::setw(width) << matrix.at(row, column) << ' ';
            }
            os << std::setw(width) << matrix.at(row, size - 1) << std::endl;
        }
        for (size_t column = 0; column < size; ++column) {
            os << std::setw(width) << matrix.at(size - 1, column) << ' ';
        }
        os << std::endl;
        return os;
    }
};
