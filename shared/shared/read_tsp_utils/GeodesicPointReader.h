#pragma once

#include <fstream>  // std::fstream
#include <vector>   // std::vector

#include "PointReader.h"
#include "geodesic_distance.h"
#include "point.h"

namespace point_reader {
	/**
	 * GeodesicPointReader implementation that reads geographic coordinates.
	 */
	class GeodesicPointReader : public PointReader {
		using point_t = point::point_geo;
		using super = PointReader;

		// this vector of geographic coordinates will be populated during the read() method
		std::vector<point_t> point_vec;

	public:
		// initialize the vector of geographic coordinates to having size = dimension
		GeodesicPointReader(std::fstream& file, size_t dimension) :
			super(file, dimension), point_vec(dimension) {
		}

		// read dimension points from the input file, and store them in point_vec
		// indexing using the vertex' label value
		void read() override {
			detail::read_point(this->file, this->dimension, point_vec);
		}

		// calculate the distance between the i-th and the j-th points
		[[nodiscard]] double distance(size_t i, size_t j) const override {
			return distance::geodesic_distance(point_vec[i], point_vec[j]);
		}
	};

}  // namespace point_reader
