#pragma once

#include <cmath>

namespace decay {
    void mutation_rate_decay(double& mutation_rate, double min_mutation_rate) {
        constexpr double a = 2;
        constexpr double b = 0.6;
        constexpr double c = 1.7;

        mutation_rate =
            std::fmin(std::log(1 + a * mutation_rate) * std::pow(b, c), min_mutation_rate);
    }
}  // namespace decay