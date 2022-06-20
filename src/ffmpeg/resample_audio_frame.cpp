#include "resample_audio_frame.hpp"

namespace FFmpeg {
    int resample_audio_frame(SwrContext* resampler, const Frame& in, Frame& out) noexcept
    {
        return swr_convert_frame(resampler, out.handle, in.handle);
    }
} // namespace FFmpeg
