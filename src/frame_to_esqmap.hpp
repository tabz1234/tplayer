#pragma once

#include <utility>

#include "ffmpeg/Frame.hpp"
#include "ptr_view.hpp"

std::string frame_to_esqmap(const ptr_view<const AVFrame> frame);
