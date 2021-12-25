#include "shrink_audio_size_to_content.hpp"

extern "C"
{
#include <libavutil/samplefmt.h>
}

void
FFmpeg::shrink_audio_size_to_content(AVFrame* aligned_frame) noexcept
{

    av_samples_get_buffer_size(aligned_frame->linesize,
                               aligned_frame->channels,
                               aligned_frame->nb_samples,
                               (AVSampleFormat)aligned_frame->format,
                               1);
}
