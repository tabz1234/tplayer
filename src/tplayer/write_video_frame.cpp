#include "write_video_frame.hpp"

#include <cstdio>

namespace tplayer {
    int write_video_frame(const FFmpeg::Frame& frame) noexcept
    {
        FILE* image_file = fopen("rgb0_image", "w");

        fwrite(frame.handle->data[0], 1, frame.handle->linesize[0] * frame.handle->height, image_file);

        fclose(image_file);

        return 0;
    }
} // namespace tplayer
