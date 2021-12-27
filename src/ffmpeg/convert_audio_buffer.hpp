#pragma once

#define __STDC_CONSTANT_MACROS
extern "C" {
#include <libavutil/avutil.h>
}

#include <list>

#include "Frame.hpp"

namespace FFmpeg {

    auto convert_audio_buffer_format(std::list<Frame<MediaType::audio>>::iterator beg,
                                     std::list<Frame<MediaType::audio>>::iterator end,
                                     const AVSampleFormat new_format) -> void;
}
