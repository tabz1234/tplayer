#pragma once

#include <string_view>

#include "../ffmpeg/Frame.hpp"

namespace tplayer {
    int write_audio_frame(const FFmpeg::Frame& frame, const std::string_view path) noexcept;
}
