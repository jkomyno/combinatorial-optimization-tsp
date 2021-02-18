#pragma once

#include <fstream>  // std::fstream
#include <vector>   // std::vector

#include "PointReader.h"
#include "euclidean_distance.h"  // distance::euclidean_distance
#include "point.h"               // point::point_2D

namespace point_reader {
    /**
     * PointReader implementation that reads 2D Euclidean points.
     */
    class EuclideanPointReader : public PointReader {
        using point_t = point::point_2D;
        using super = PointReader;

        // this vector of 2D Euclidean points will be populated during the read() method
        std::vector<point_t> point_vec;

    public:
        // initialize the vector of 2D Euclidean points to having size = dimension
        EuclideanPointReader(std::fstream& file, size_t dimension) :
            super(file, dimension), point_vec(dimension) {
        }

        // read dimension points from the input file, and store them in point_vec
        // indexing using the vertex' label value
        void read() override {
            detail::read_point(this->file, this->dimension, point_vec);
        }

        // calculate the distance between the i-th and the j-th points
        [[nodiscard]] double distance(size_t i, size_t j) const override {
            return distance::euclidean_distance(point_vec[i], point_vec[j]);
        }
    };
}  // namespace point_reader
