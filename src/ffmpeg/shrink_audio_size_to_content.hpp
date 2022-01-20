#pragma once

extern "C" {
#include <libavutil/frame.h>
}

#include "../ptr_view.hpp"

namespace FFmpeg {

    void shrink_audio_size_to_content(const ptr_view<AVFrame> aligned_frame) noexcept;

}
