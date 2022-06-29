#include "write_video_frame.hpp"

#include <cstdio>

namespace tplayer {
    int write_video_frame(const FFmpeg::Frame& frame, const std::string_view path) noexcept
    {
        FILE* image_file = fopen(path.data(), "wb");
        if (!image_file) [[unlikely]] {
            return -1;
        }

        fwrite(frame.handle->data[0], 1, frame.handle->linesize[0] * frame.handle->height, image_file);

        fclose(image_file);

        return 0;
    }
} // namespace tplayer
