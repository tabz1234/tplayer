#include "convert_audio_buffer.hpp"

extern "C"
{
#include <libswresample/swresample.h>
}

#include <stdexcept>

void
FFmpeg::convert_audio_buffer_format(std::list<Frame<AVMEDIA_TYPE_AUDIO>>::iterator beg,
                                    std::list<Frame<AVMEDIA_TYPE_AUDIO>>::iterator end,
                                    const AVSampleFormat new_format)
{

    SwrContext* swr_ctx_ = swr_alloc_set_opts(nullptr,

                                              beg->ptr()->channel_layout,
                                              new_format,
                                              beg->ptr()->sample_rate,

                                              beg->ptr()->channel_layout,
                                              (AVSampleFormat)beg->ptr()->format,
                                              beg->ptr()->sample_rate,

                                              0,
                                              nullptr);

    if (swr_ctx_ == nullptr) [[unlikely]] {
        throw std::runtime_error("swr_alloc_set_opts failed");
    }

    int c_api_ret = swr_init(swr_ctx_);
    if (c_api_ret < 0) [[unlikely]] {
        throw std::runtime_error("swr_init failed");
    }

    for (auto& it = beg; it != end; ++it) {
        Frame<AVMEDIA_TYPE_AUDIO> resampled_frame;

        resampled_frame.ptr()->sample_rate = it->ptr()->sample_rate;
        resampled_frame.ptr()->channel_layout = it->ptr()->channel_layout;
        resampled_frame.ptr()->format = new_format;
        resampled_frame.ptr()->nb_samples = it->ptr()->nb_samples;

        av_frame_copy_props(resampled_frame.ptr(), it->ptr());

        c_api_ret = swr_convert_frame(swr_ctx_, resampled_frame.ptr(), it->ptr());

        if (c_api_ret != 0) [[unlikely]] {
            throw std::runtime_error("swr_convert_frame failed");
        }

        *it = std::move(resampled_frame);
    }

    swr_close(swr_ctx_);
    swr_free(&swr_ctx_);
}
