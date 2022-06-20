#pragma once

#include "Frame.hpp"
#include "SWScaler.hpp"

namespace FFmpeg {
    int scale_video_frame(SwsContext* scaler, const Frame& in, Frame& out) noexcept;
}
