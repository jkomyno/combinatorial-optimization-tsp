#pragma once

#include <vector>  // std::vector

// PermutationPath represents a TSP path as a permutation of a vector [0, 1, ..., n-1], where n is
// the number of cities in the problem.
// The type T indicates the type of the distance between each two cities.
// TODO: implement lazy cost function
template <typename T = double>
class PermutationPath {
    using path_t = std::vector<size_t>;
    using iterator = path_t::iterator;
    using const_iterator = path_t::const_iterator;

    // It contains the path represented as a permutation of [0, 1, ..., n - 1]
    path_t path;

public:
    explicit PermutationPath(const path_t& path) : path(path) {
    }

    explicit PermutationPath(path_t&& path) : path(std::move(path)) {
    }

    // Copy constructor
    explicit PermutationPath(const PermutationPath<T>& other) noexcept {
        this->path = other.path;
    }

    // Move constructor
    explicit PermutationPath(PermutationPath<T>&& other) noexcept {
        using std::swap;
        swap(this->path, other.path);
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
    }
};
