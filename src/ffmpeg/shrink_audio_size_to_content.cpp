#include "shrink_audio_size_to_content.hpp"

extern "C" {
#include <libavutil/samplefmt.h>
}

#include "../check.hpp"

void FFmpeg::shrink_audio_size_to_content(AVFrame* const aligned_frame) noexcept
{

    const auto c_api_ret =
        av_samples_get_buffer_size(aligned_frame->linesize,
                                   aligned_frame->channels,
                                   aligned_frame->nb_samples,
                                   static_cast<enum AVSampleFormat>(aligned_frame->format),
                                   1);
    check(c_api_ret >= 0, "av_samples_get_buffer_size failed");
}
