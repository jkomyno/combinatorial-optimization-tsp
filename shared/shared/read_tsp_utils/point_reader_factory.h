#pragma once

// suppress warning C4715: not all control paths return a value
#if defined(_MSC_VER)
#pragma warning(disable : 4715)
#endif

#include <fstream>  // std::fstream

#include "EdgeWeightType.h"

#include "EuclideanPointReader.h"
#include "PointReader.h"

namespace point_reader {
    // Only Euclidean distance is allowed.
    std::unique_ptr<PointReader> point_reader_factory(EdgeWeightType edge_weight_type,
                                                      std::fstream& file,
                                                      size_t dimension) noexcept {
        switch (edge_weight_type) {
        case EdgeWeightType::EUC_2D:
            return std::make_unique<EuclideanPointReader>(file, dimension);
        }
    }
}  // namespace point_reader
