#pragma once

#include <iostream>   // std::istream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string_literals

// enumeration of the supported metric distances
enum class EdgeWeightType {
    EUC_2D,  // euclidean 2-dimensional distance
    GEO      // geodesic distance
};

// read a EdgeWeightType object in input
std::istream& operator>>(std::istream& is, EdgeWeightType& edge_weight_type) {
    std::string name;
    is >> name;

    if (name == "EUC_2D") {
        edge_weight_type = EdgeWeightType::EUC_2D;
    } else if (name == "GEO") {
        edge_weight_type = EdgeWeightType::GEO;
    } else {
        using namespace std::string_literals;
        throw std::runtime_error("Unsupported EDGE_WEIGHT_TYPE"s);
    }

    return is;
}
