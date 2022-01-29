#pragma once

extern "C" {
#include "libavutil/rational.h"
}

#include <chrono>
#include <concepts>

namespace FFmpeg {
    template <std::integral T>
    auto apply_time_ratio(const T val, const AVRational time_ratio) noexcept
    { //@TODO try fractions array
        return std::chrono::duration<long double>{val * time_ratio.num /
                                                  static_cast<long double>(time_ratio.den)};
    }

} // namespace FFmpeg
