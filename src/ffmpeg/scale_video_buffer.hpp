#pragma once

#include <list>

#include "Frame.hpp"

namespace FFmpeg {
    void scale_video_buffer(std::list<FFmpeg::Frame<FFmpeg::MediaType::video>>::iterator begin,
                            std::list<FFmpeg::Frame<FFmpeg::MediaType::video>>::iterator end,
                            int new_width,
                            int new_height);
    // output is always in RGB format
} // namespace FFmpeg
