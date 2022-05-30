#pragma once

#include "ffmpeg/Frame.hpp"

std::string frame_to_esqmap(const AVFrame* const frame);
