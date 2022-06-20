#pragma once

extern "C" {
#include <libswresample/swresample.h>
}

namespace FFmpeg {
    SwrContext* create_swr_resampler(const int64_t in_ch_layout,
                                     const enum AVSampleFormat in_sample_fmt,
                                     const int in_sample_rate,
                                     const int64_t out_ch_layout,
                                     const enum AVSampleFormat out_sample_fmt,
                                     const int out_sample_rate) noexcept;

    void free_swr_resampler(SwrContext* handle) noexcept;
} // namespace FFmpeg
