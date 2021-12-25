#pragma once

extern "C"
{
#include <libavutil/frame.h>
}

namespace FFmpeg {

void
shrink_audio_size_to_content(AVFrame* aligned_frame) noexcept;

}

