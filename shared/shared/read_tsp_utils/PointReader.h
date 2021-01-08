#pragma once

#include <fstream>     // std::fstream
#include <functional>  // std::bind
#include <vector>      // std::vector

#include "../DistanceMatrix.h"
#include "point.h"

namespace point_reader {
    /**
     *
     */
    class PointReader {
    protected:
        std::fstream& file;

        // calculate the distance between the i-th and the j-th points
        virtual double distance(size_t i, size_t j) const = 0;

    public:
        const size_t dimension;

        PointReader(std::fstream& file, size_t dimension) : file(file), dimension(dimension) {
        }

        virtual ~PointReader() = default;

        // consume the list of points from the input file
        virtual void read() = 0;

        // create a distance matrix after reading the points. It uses the distance method
        // implemented by the child classes as an higher-order function
        DistanceMatrix<double> create_distance_matrix() {
            using namespace std::placeholders;

            // _1 and _2 indicate that the function receives 2 arguments.
            auto distance_fun(std::bind(&PointReader::distance, this, _1, _2));

            return DistanceMatrix<double>(dimension, distance_fun);
        }
    };

    namespace detail {
        /**
         * Shared implementation of PointReader's read() method.
         * This can't be directly embed it in PointReader because can't know a priori the the type
         * associated with the vector point_vec. PointReader can't be generic either, because
         * otherwise we wouldn't know how to instantiate it (it would have two different generic
         * types for GEO and EUC_2D points).
         */
        template <class Point>
        void read_point(std::fstream& file, size_t dimension, std::vector<Point>& point_vec) {
            size_t n;  // label of the vertex
            Point p;   // generic point of the type required by the child class which called this
                       // method
            for (size_t i = 0; i < dimension; ++i) {
                file >> n >> p;

                // we decrease by 1 the value of the vertex for comfortableness reasons
                point_vec.at(n - 1) = p;
            }
        }
    }  // namespace detail
}  // namespace point_reader
