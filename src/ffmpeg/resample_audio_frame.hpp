#pragma once

#include "Frame.hpp"
#include "SWResampler.hpp"

namespace FFmpeg {
    int resample_audio_frame(SwrContext* resampler, const Frame& in, Frame& out) noexcept;
}
