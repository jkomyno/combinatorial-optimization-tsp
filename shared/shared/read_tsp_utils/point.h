#pragma once

#include <iostream>  // std::istream

#include "utils.h"  // utils::to_radians

namespace point {
    // 2-dimensional point in the Euclidean space
    struct point_2D {
        double x;
        double y;

        point_2D() = default;

        point_2D(double x, double y) : x(x), y(y) {
        }
    };

    // 2-dimensional point expressed in terms of latitude and longitude.
    // The coordinates are converted to radians.
    struct point_geo {
        double latitude;
        double longitude;

        point_geo() = default;

        point_geo(double latitude, double longitude) :
            latitude(utils::to_radians(latitude)),
            longitude(utils::to_radians(longitude)) {
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

// read a point_geo object in input
inline std::istream& operator>>(std::istream& is, point::point_geo& point) {
    double latitude;
    double longitude;

    is >> latitude >> longitude;
    point = point::point_geo(latitude, longitude);

    return is;
}
