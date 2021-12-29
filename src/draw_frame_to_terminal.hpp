#pragma once

#include <utility>

#include "ffmpeg/Frame.hpp"

void draw_frame_to_terminal(FFmpeg::Frame<FFmpeg::MediaType::video> frame,
                            const std::pair<int, int> position) noexcept;
