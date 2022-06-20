#include "SWResampler.hpp"

namespace FFmpeg {
    SwrContext* create_swr_resampler(const int64_t in_ch_layout,
                                     const enum AVSampleFormat in_sample_fmt,
                                     const int in_sample_rate,
                                     const int64_t out_ch_layout,
                                     const enum AVSampleFormat out_sample_fmt,
                                     const int out_sample_rate) noexcept
    {
        SwrContext* ret = swr_alloc_set_opts(nullptr,
                                             out_ch_layout,
                                             out_sample_fmt,
                                             out_sample_rate,
                                             in_ch_layout,
                                             in_sample_fmt,
                                             in_sample_rate,
                                             0,
                                             nullptr);
        swr_init(ret);

        return ret;
    }
    void free_swr_resampler(SwrContext* handle) noexcept
    {
        swr_close(handle);
        swr_free(&handle);
    }
} // namespace FFmpeg
