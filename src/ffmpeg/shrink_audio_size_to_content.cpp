#include "shrink_audio_size_to_content.hpp"

extern "C" {
#include <libavutil/samplefmt.h>
}

#include "../util/fcheck.hpp"

namespace FFmpeg {
    void shrink_audio_size_to_content(Frame& aligned_frame) noexcept
    {
        const auto c_api_ret = av_samples_get_buffer_size(aligned_frame.handle->linesize,
                                                          aligned_frame.handle->channels,
                                                          aligned_frame.handle->nb_samples,
                                                          static_cast<enum AVSampleFormat>(aligned_frame.handle->format),
                                                          1);
        fcheck(c_api_ret == 0, [] {
            fprintf(stderr, "av_samples_get_buffer_size failed\n");
        });
    }
} // namespace FFmpeg
