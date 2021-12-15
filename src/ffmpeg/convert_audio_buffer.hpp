#ifndef FFMPEG_CONVERT_AUDIO_BUFFER_HPP
#define FFMPEG_CONVERT_AUDIO_BUFFER_HPP

#define __STDC_CONSTANT_MACROS
extern "C"
{
#include <libavutil/avutil.h>
}

#include <list>

#include "Frame.hpp"

namespace FFmpeg {

void
convert_audio_buffer_format(std::list<Frame<AVMEDIA_TYPE_AUDIO>>::iterator beg,
                            std::list<Frame<AVMEDIA_TYPE_AUDIO>>::iterator end,
                            const AVSampleFormat new_format);
}

#endif
