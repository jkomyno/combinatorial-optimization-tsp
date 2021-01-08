#pragma once

#include <chrono>  // std::chrono

/**
 * std::chrono wrapper that ensure we consistently use the same clock around the projects' files
 */
namespace stopwatch {
    // monotonic clock
    using clock_t = std::chrono::steady_clock;

    // represents a moment in time
    using time_point_t = clock_t::time_point;

    // microseconds
    using us_t = std::chrono::microseconds;

    // milliseconds
    using ms_t = std::chrono::milliseconds;

    [[nodiscard]] inline time_point_t now() noexcept {
        return clock_t::now();
    }

    template <typename Duration>
    [[nodiscard]] auto duration(time_point_t start_time, time_point_t stop_time) noexcept {
        return std::chrono::duration_cast<Duration>(stop_time - start_time).count();
    }
}  // namespace stopwatch
