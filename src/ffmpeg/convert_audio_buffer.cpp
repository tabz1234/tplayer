#include "convert_audio_buffer.hpp"

extern "C" {
#include <libswresample/swresample.h>
}

#include <execution>
#include <stdexcept>

#include "../check.hpp"

std::vector<FFmpeg::Frame<FFmpeg::MediaType::audio>>
FFmpeg::convert_audio_buffer_format(const std::vector<Frame<MediaType::audio>>::const_iterator begin,
                                    const std::vector<Frame<MediaType::audio>>::const_iterator end,
                                    const AVSampleFormat new_format)
{

    SwrContext* swr_ctx_ = swr_alloc_set_opts(nullptr,

                                              begin->ptr()->channel_layout,
                                              new_format,
                                              begin->ptr()->sample_rate,

                                              begin->ptr()->channel_layout,
                                              static_cast<enum AVSampleFormat>(begin->ptr()->format),
                                              begin->ptr()->sample_rate,

                                              0,
                                              nullptr);

    check(swr_ctx_ != nullptr, " swr_alloc_set_opts failed");

    auto c_api_ret = swr_init(swr_ctx_);
    check(c_api_ret >= 0, "swr_init failed");

    std::vector<Frame<MediaType::audio>> out_vec;
    out_vec.reserve(end - begin);

    for (auto i = begin; i != end; ++i) {
        Frame<MediaType::audio> resampled_frame;

        resampled_frame.ptr()->sample_rate = i->ptr()->sample_rate;
        resampled_frame.ptr()->channel_layout = i->ptr()->channel_layout;
        resampled_frame.ptr()->format = new_format;
        resampled_frame.ptr()->nb_samples = i->ptr()->nb_samples;

        av_frame_copy_props(resampled_frame.ptr(), i->ptr());

        c_api_ret = swr_convert_frame(swr_ctx_, resampled_frame.ptr(), i->ptr());
        check(c_api_ret == 0, "swr_convert_frame failed");

        out_vec.emplace_back(std::move(resampled_frame));
    }

    swr_close(swr_ctx_);
    swr_free(&swr_ctx_);

    return out_vec;
}
