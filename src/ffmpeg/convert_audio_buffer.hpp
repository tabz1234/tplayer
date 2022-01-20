#pragma once

#define __STDC_CONSTANT_MACROS
extern "C" {
#include <libavutil/avutil.h>
}

#include <vector>

#include "Frame.hpp"

namespace FFmpeg {

    std::vector<Frame<MediaType::audio>>
    convert_audio_buffer_format(const std::vector<Frame<MediaType::audio>>::const_iterator begin,
                                const std::vector<Frame<MediaType::audio>>::const_iterator end,
                                const AVSampleFormat new_format);
}
