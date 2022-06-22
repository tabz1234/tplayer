#include "write_video_frame.hpp"

#include <cstdio>

namespace tplayer {
    int write_audio_frame(const FFmpeg::Frame& frame) noexcept
    {
        FILE* sample_file = fopen("FLT_data", "w");

        fwrite(frame.handle->data[0], 1, frame.handle->linesize[0], sample_file);

        fclose(sample_file);

        return 0;
    }
} // namespace tplayer
