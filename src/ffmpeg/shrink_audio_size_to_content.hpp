#pragma once

#include "Frame.hpp"

namespace FFmpeg {
    int shrink_audio_size_to_content(Frame& aligned_frame) noexcept;
} // namespace FFmpeg
