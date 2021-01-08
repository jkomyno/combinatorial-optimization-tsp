#pragma once

#include <cmath>  // std::pow, std::sqrt, std::round

#include "point.h"

namespace distance {
	// return the Euclidean distance between point i and point j
	[[nodiscard]] inline double euclidean_distance(const point::point_2D& i,
		const point::point_2D& j) noexcept {
		const auto& [x_i, y_i] = i;
		const auto& [x_j, y_j] = j;

		const double x = x_i - x_j;
		const double y = y_i - y_j;

		// distance is the Euclidean distance between points i and j
		const double distance = std::sqrt(std::pow(x, 2) + std::pow(y, 2));

		// round to the nearest integer value
		return std::round(distance);
	}
}  // namespace distance
