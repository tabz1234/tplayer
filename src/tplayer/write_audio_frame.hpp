#pragma once

#include "../ffmpeg/Frame.hpp"

namespace tplayer {
    int write_audio_frame(const FFmpeg::Frame& frame) noexcept;
}
