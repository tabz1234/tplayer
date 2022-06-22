#include "resample_audio_frame.hpp"

namespace FFmpeg {
    int resample_audio_frame(SwrContext* resampler, const Frame& in, Frame& out) noexcept
    {
        out.handle->sample_rate = in.handle->sample_rate;
        out.handle->channel_layout = in.handle->channel_layout;
        out.handle->nb_samples = in.handle->nb_samples;

        av_frame_copy_props(out.handle, in.handle);

        return swr_convert_frame(resampler, out.handle, in.handle);
    }
} // namespace FFmpeg
