#ifndef FFMPEG_AUDIO_FORMAT_CONVERTER_HPP
#define FFMPEG_AUDIO_FORMAT_CONVERTER_HPP

#define __STDC_CONSTANT_MACROS
extern "C"
{
#include <libavutil/avutil.h>
}

#include <list>

#include "FFmpegRaiiFrame.hpp"

namespace ffmpeg {

void
convert_audio_buffer_format(std::list<RaiiFrame<AVMEDIA_TYPE_AUDIO>>::iterator beg,
                            std::list<RaiiFrame<AVMEDIA_TYPE_AUDIO>>::iterator end,
                            const AVSampleFormat new_format);
}

#endif
