#pragma once

#include <iostream>

namespace solver::table {
    static constexpr auto SEP = "\t|\t";

    std::ostream& header(std::ostream& out) noexcept {
        // verbose output
        out << "Gen #i  | best_(i-1)    | best_i        | avg_cost      | "
               "generations_without_improvement"
            << '\n';
        return out;
    }

    std::ostream& row(std::ostream& out, size_t n_generations, size_t previous_best_cost,
                      size_t current_best_cost, size_t avg_cost,
                      size_t n_generations_without_improvement) noexcept {
        out << '#' << n_generations << SEP << previous_best_cost << SEP << current_best_cost << SEP
            << avg_cost << SEP << n_generations_without_improvement << '\n'
            << std::flush;
        return out;
    }
}  // namespace solver::table