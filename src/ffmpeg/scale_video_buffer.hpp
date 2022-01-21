#pragma once

extern "C" {
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

#include <vector>

#include "Frame.hpp"

namespace FFmpeg {

    template <typename ContainerT> //@TODO template template parameter
    std::vector<Frame<MediaType::video>>
    scale_video_buffer(const typename ContainerT::const_iterator begin,
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

        std::vector<Frame<MediaType::video>> out_vec;

        std::for_each(begin, end, [&](const auto& input_frame) { //@TODO try paralel
            Frame<MediaType::video> converted_frame;

            converted_frame.ptr()->width = new_width;
            converted_frame.ptr()->height = new_height;
            converted_frame.ptr()->format = new_format;

            av_frame_copy_props(converted_frame.ptr(), input_frame.ptr());

            const auto c_api_ret = av_frame_get_buffer(converted_frame.ptr(), 0);
            check(c_api_ret == 0, "av_frame_get_buffer failed");

            sws_scale(sws_ctx,
                      input_frame.ptr()->data,
                      input_frame.ptr()->linesize,
                      0,
                      input_frame.ptr()->height,
                      converted_frame.ptr()->data,
                      converted_frame.ptr()->linesize);

            out_vec.emplace_back(std::move(converted_frame));
        });

        sws_freeContext(sws_ctx);

        return out_vec;
    }

} // namespace FFmpeg
