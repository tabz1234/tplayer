#pragma once

#include "../ffmpeg/Frame.hpp"

namespace tplayer {
    int write_video_frame(const FFmpeg::Frame& frame) noexcept;
}
