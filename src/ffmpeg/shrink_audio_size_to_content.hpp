#pragma once

extern "C" {
#include <libavutil/frame.h>
}

#include "Frame.hpp"

namespace FFmpeg {
    void shrink_audio_size_to_content(Frame& aligned_frame) noexcept;
} // namespace FFmpeg
