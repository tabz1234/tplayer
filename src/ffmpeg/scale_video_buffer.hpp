#pragma once

extern "C" {
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

#include <vector>

#include "Frame.hpp"

namespace FFmpeg {

    template <typename ContainerT>
    ContainerT scale_video_buffer(const typename ContainerT::const_iterator begin,
                                  const typename ContainerT::const_iterator end,
                                  const int new_width,
                                  const int new_height,
                                  const enum AVPixelFormat new_format = AV_PIX_FMT_RGB0)
    {

        const auto sws_ctx = sws_getContext(begin->ptr()->width,
                                            begin->ptr()->height,
                                            static_cast<enum AVPixelFormat>(begin->ptr()->format),
                                            new_width,
                                            new_height,
                                            new_format,
                                            SWS_BICUBIC,
                                            nullptr,
                                            nullptr,
                                            nullptr);

        check(sws_ctx != nullptr, "sws_getContext failed");

        ContainerT out_vec;
        auto out_it = out_vec.begin();

        std::for_each(begin, end, [&](const auto& it) { //@TODO try paralel
            Frame<MediaType::video> converted_frame;

            converted_frame.ptr()->width = new_width;
            converted_frame.ptr()->height = new_height;
            converted_frame.ptr()->format = new_format;

            const auto c_api_ret = av_frame_get_buffer(converted_frame.ptr(), 0);
            check(c_api_ret == 0, "av_frame_get_buffer failed");

            sws_scale(sws_ctx,
                      it.ptr()->data,
                      it.ptr()->linesize,
                      0,
                      it.ptr()->height,
                      converted_frame.ptr()->data,
                      converted_frame.ptr()->linesize);

            *out_it = std::move(converted_frame);
            ++out_it;
        });

        sws_freeContext(sws_ctx);

        return out_vec;
    }

} // namespace FFmpeg
