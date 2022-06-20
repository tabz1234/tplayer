#include "shrink_audio_size_to_content.hpp"

extern "C" {
#include <libavutil/samplefmt.h>
}

namespace FFmpeg {
    int shrink_audio_size_to_content(Frame& aligned_frame) noexcept
    {
        const auto ret = av_samples_get_buffer_size(aligned_frame.handle->linesize,
                                                    aligned_frame.handle->channels,
                                                    aligned_frame.handle->nb_samples,
                                                    static_cast<enum AVSampleFormat>(aligned_frame.handle->format),
                                                    1);
        return ret;
    }
} // namespace FFmpeg
