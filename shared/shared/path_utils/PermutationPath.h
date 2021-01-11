#pragma once

#include <limits>  // std::numeric_limits
#include <vector>  // std::vector

#include "../DistanceMatrix.h"
#include "utils.h"

// PermutationPath represents a TSP path as a permutation of a vector [0, 1, ..., n-1], where n is
// the number of cities in the problem.
// All copy-assignments and move-assignments between different PermutationPath must refer to the
// same common DistanceMatrix.
// The type T indicates the type of the distance between each two cities.
template <typename T = double>
class PermutationPath {
    using path_t = std::vector<size_t>;
    using iterator = path_t::iterator;
    using const_iterator = path_t::const_iterator;

    // Initial circuit distance of this path. Since the distance for the same solution might
    // get compute multiple times and it's quite expensive (linear to the size of the path),
    // it is computed lazily and memoized for future calls to PermutationPath<T>::cost().
    static constexpr T DISTANCE_NOT_COMPUTED = std::numeric_limits<T>::max();

    // It contains the path represented as a permutation of [0, 1, ..., n - 1]
    path_t path;

    // Contant reference to the distance matrix of this permutation path
    const DistanceMatrix<T>& distance_matrix;

    // Circuit distance of the path: δ(path[0], path[1]) + ... + δ(path[n - 1], path[0]).
    // distance is mutable so that PermutationPath<T>::cost() is a const method.
    mutable T distance = DISTANCE_NOT_COMPUTED;

    // Return the distance between any 2 nodes
    T get_distance_helper(const size_t x, const size_t y) const {
        return this->distance_matrix.at(x, y);
    }

public:
    template <typename V>
    static PermutationPath<V> from_size(size_t n,
                                        const DistanceMatrix<V>& distance_matrix) noexcept {
        return PermutationPath<V>(utils::vector_in_range(0, n), distance_matrix);
    }

    explicit PermutationPath(const path_t& path, const DistanceMatrix<T>& distance_matrix) :
        path(path),
        distance_matrix(distance_matrix) {
    }

    explicit PermutationPath(path_t&& path, const DistanceMatrix<T>& distance_matrix) :
        path(std::move(path)),
        distance_matrix(distance_matrix) {
    }

    // Copy constructor
    PermutationPath(const PermutationPath<T>& other) noexcept :
        path(other.path),
        distance_matrix(other.distance_matrix),
        distance(other.distance) {
    }

    // Move constructor
    PermutationPath(PermutationPath<T>&& other) noexcept :
        path(std::move(other.path)),
        distance_matrix(other.distance_matrix),
        distance(other.distance) {
    }

    PermutationPath<T>& operator=(const PermutationPath<T>& other) {
        if (this != &other && &(this->distance_matrix) == &(other.distance_matrix)) {
            this->path = other.path;
            this->distance = other.distance;
        }

        return *this;
    }

    PermutationPath<T>& operator=(PermutationPath<T>&& other) {
        if (this != &other && &(this->distance_matrix) == &(other.distance_matrix)) {
            this->path = std::move(other.path);
            this->distance = other.distance;
        }

        return *this;
    }

    // Lazily compute the total cost of the solution and memoize it for future reference.
    [[nodiscard]] T cost() const noexcept {
        if (this->distance == DISTANCE_NOT_COMPUTED) {
            auto get_distance = [this](const size_t x, const size_t y) -> T {
                return this->get_distance_helper(x, y);
            };

            // Compute the circuit distance
            this->distance =
                utils::sum_distances_as_circuit(this->cbegin(), this->cend(), get_distance);
        }

        return this->distance;
    }

    // Reset cost.
    // TODO: add way to compute updated cost after mutation without recomputing everything.
    void reset_cost() noexcept {
        this->distance = DISTANCE_NOT_COMPUTED;
    }

    /**
     * Propagate std::vector most important methods:
     * - iterators
     * - size()
     * - []
     * - swap
     */

    iterator begin() noexcept {
        return path.begin();
    }

    iterator end() noexcept {
        return path.end();
    }

    const_iterator begin() const noexcept {
        return path.begin();
    }

    const_iterator end() const noexcept {
        return path.end();
    }

    const_iterator cbegin() const noexcept {
        return path.cbegin();
    }

    const_iterator cend() const noexcept {
        return path.cend();
    }

    size_t& operator[](size_t i) {
        return path[i];
    }

    const size_t& operator[](size_t i) const {
        return path[i];
    }

    size_t size() const noexcept {
        return path.size();
    }

    template <typename V>
    friend void swap(PermutationPath<V>& a, PermutationPath<V>& b) {
        using std::swap;
        swap(a.path, b.path);
        swap(a.distance, b.distance);
    }
};
