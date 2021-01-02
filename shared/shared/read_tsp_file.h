#pragma once

#include <fstream>    // std::fstream, std::fstream
#include <limits>     // std::numeric_limits
#include <memory>     // std::unique_ptr
#include <stdexcept>  // std::exception
#include <string>     // std::string, std::string_literals

#include "EdgeWeightType.h"
#include "point_reader_factory.h"
#include "shared_utils.h"

// skip n_skip chars while reading from file
inline void skip_chars(std::fstream& file,
                       std::streamsize n_skip = std::numeric_limits<std::streamsize>::max(),
                       const char delimiter = '\n') {
    file.ignore(n_skip, delimiter);
}

// skip a line while reading from file
inline void skip_line(std::fstream& file) {
    skip_chars(file);
}

// read a TSP file definition, skipping the unimportant header parts
inline std::unique_ptr<point_reader::PointReader> read_tsp_file(const char* filename) {
    using namespace std::string_literals;

    std::fstream file(filename);
    if (!file.good()) {
        throw std::runtime_error("File doesn't exist"s);
    }

    skip_line(file);  // skip NAME line
    skip_line(file);  // skip TYPE/COMMENT line
    skip_line(file);  // skip COMMENT/TYPE line

    skip_chars(file, utils::string_length("DIMENSION:"));

    // read dimension of the graph
    size_t dimension;
    file >> dimension;
    skip_line(file);  // skip to next line after reading dimension

    skip_chars(file, utils::string_length("EDGE_WEIGHT_TYPE:"));

    // read type of coordinates. The distance measure will depend on this would
    EdgeWeightType edge_weight_type;
    file >> edge_weight_type;
    skip_line(file);  // skip to next line after reading edge_weight_type

    // skip lines until "NODE_COORD_SECTION" string
    std::string line;
    bool stop = false;
    while (!stop) {
        std::getline(file, line);
        if (line.find("NODE_COORD_SECTION") == 0) {
            stop = true;
        }
    }

    // point_reader knows how to read the points according to their type (determined by
    // edge_weight_type). After reading the points, it creates the distance matrix using the correct
    // distance function, i.e. the geodesic distance for GEO points, and euclidean distance for
    // EUC_2D.
    auto point_reader(point_reader::point_reader_factory(edge_weight_type, file, dimension));
    point_reader->read();

    // release file
    file.close();
    return point_reader;
}
