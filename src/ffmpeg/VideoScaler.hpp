#pragma once

extern "C" {
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

#include "Frame.hpp"

namespace FFmpeg {

    struct VideoScaler {

        VideoScaler(const int old_width,
                    const int old_height,
                    const int old_format,
                    const int new_width,
                    const int new_height,
                    const int = AV_PIX_FMT_RGB0);

        Frame<MediaType::video> scale(const Frame<MediaType::video>& input_frame);

        ~VideoScaler();

      private:
        SwsContext* sws_ctx_;

        const int new_width_;
        const int new_height_;
        const int new_format_;

      public:
        VideoScaler(const VideoScaler&) = delete;
        VideoScaler& operator=(const VideoScaler&) = delete;

        VideoScaler(VideoScaler&&) noexcept = delete;
        VideoScaler& operator=(VideoScaler&&) noexcept = delete;
    };

} // namespace FFmpeg
