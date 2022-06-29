#pragma once

extern "C" {
#include <libavutil/rational.h>
}

namespace FFmpeg {
    template <typename T>
    long double apply_ratio(const T val, const AVRational ratio) noexcept
    {
        return val * ratio.num / static_cast<long double>(ratio.den);
    }
} // namespace FFmpeg
