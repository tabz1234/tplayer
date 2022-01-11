#include "scale_video_buffer.hpp"

extern "C" {
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

#include <execution>

void FFmpeg::scale_video_buffer(std::list<FFmpeg::Frame<FFmpeg::MediaType::video>>::iterator begin,
                                std::list<FFmpeg::Frame<FFmpeg::MediaType::video>>::iterator end,
                                int new_width,
                                int new_height) {

    const auto sws_ctx = sws_getContext(begin->ptr()->width,
                                        begin->ptr()->height,
                                        (AVPixelFormat)begin->ptr()->format,
                                        new_width,
                                        new_height,
                                        AV_PIX_FMT_RGB0,
                                        SWS_FAST_BILINEAR,
                                        nullptr,
                                        nullptr,
                                        nullptr);

    check(sws_ctx != nullptr, "sws_getContext failed");

    std::for_each(std::execution::par, begin, end, [&](auto& it) {
        sws_scale(sws_ctx,
                  it.ptr()->data,
                  it.ptr()->linesize,
                  0,
                  it.ptr()->height,
                  it.ptr()->data,
                  it.ptr()->linesize);
    });

    sws_freeContext(sws_ctx);
}