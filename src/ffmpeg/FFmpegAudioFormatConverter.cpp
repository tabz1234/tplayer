#include "FFmpegAudioFormatConverter.hpp"

extern "C"
{
#include <libswresample/swresample.h>
}

#include <stdexcept>
#include <stdlib.h>

void
ffmpeg::convert_audio_buffer_format(std::list<RaiiFrame<AVMEDIA_TYPE_AUDIO>>::iterator beg,
                                    std::list<RaiiFrame<AVMEDIA_TYPE_AUDIO>>::iterator end,
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
        RaiiFrame<AVMEDIA_TYPE_AUDIO> resampled_frame;

        resampled_frame.ptr()->sample_rate = it->ptr()->sample_rate;
        resampled_frame.ptr()->channel_layout = it->ptr()->channel_layout;
        resampled_frame.ptr()->format = new_format;

        av_frame_copy_props(resampled_frame.ptr(), it->ptr());

        c_api_ret = swr_convert_frame(swr_ctx_, resampled_frame.ptr(), it->ptr());

        int i;
        for (i = resampled_frame.ptr()->linesize[0] - 1; i >= 0; --i) {
            if (resampled_frame.ptr()->data[0][i] != (uint8_t)0)
                break;
        }
        resampled_frame.ptr()->linesize[0] = i + 1;
        /*note : ffmpeg leave data with some empty cells (for alignment) filled with 0*/

        if (c_api_ret != 0) [[unlikely]] {
            throw std::runtime_error("swr_convert_frame failed");
        }

        *it = std::move(resampled_frame);
    }

    swr_close(swr_ctx_);
    swr_free(&swr_ctx_);
}
