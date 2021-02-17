#pragma once

namespace utils {
    // Determine length of string at compile time
    template <size_t N>
    constexpr auto string_length(char const (&)[N]) {
        return N;
    }
}  // namespace utils