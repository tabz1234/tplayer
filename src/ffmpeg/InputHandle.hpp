#pragma once

#include <filesystem>

extern "C" {
#include "libavformat/avformat.h"
}

namespace FFmpeg {

    struct InputHandle final {

        InputHandle(const std::filesystem::path& filepath);

        AVFormatContext* get_ptr() noexcept;

        ~InputHandle();

      private:
        AVFormatContext* av_format_ctx_;

      public:
        InputHandle(const InputHandle&) = delete;
        InputHandle& operator=(const InputHandle&) = delete;

        InputHandle(const InputHandle&&) noexcept = delete;
        InputHandle& operator=(const InputHandle&&) noexcept = delete;
    };

} // namespace FFmpeg
