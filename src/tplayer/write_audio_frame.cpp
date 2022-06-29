#include "write_video_frame.hpp"

#include <cstdio>

namespace tplayer {
    int write_audio_frame(const FFmpeg::Frame& frame, const std::string_view path) noexcept
    {
        FILE* sample_file = fopen(path.data(), "wb");
        if (!sample_file) [[unlikely]] {
            return -1;
        }

        fwrite(frame.handle->data[0], 1, frame.handle->linesize[0], sample_file);

        fclose(sample_file);

        return 0;
    }
} // namespace tplayer
