#ifndef FFMPEG_SHRINK_AUDIO_SIZE_TO_CONTENT_HPP
#define FFMPEG_SHRINK_AUDIO_SIZE_TO_CONTENT_HPP

extern "C"
{
#include <libavutil/frame.h>
}

namespace FFmpeg {

void
shrink_audio_size_to_content(AVFrame* aligned_frame);

}

#endif
