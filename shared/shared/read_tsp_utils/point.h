#pragma once

#include <iostream>  // std::istream

namespace point {
    // 2-dimensional point in the Euclidean space
    struct point_2D {
        double x;
        double y;

        point_2D() = default;

        point_2D(double x, double y) : x(x), y(y) {
        }
    };
}  // namespace point

// read a point_2D object in input
inline std::istream& operator>>(std::istream& is, point::point_2D& point) {
    double x;
    double y;

    is >> x >> y;
    point = point::point_2D(x, y);

    return is;
}
