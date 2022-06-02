#pragma once

extern "C" {
#include <libavformat/avformat.h>
}

namespace FFmpeg {
    struct FormatHandle final {

        AVFormatContext* handle;

        FormatHandle() noexcept;
        bool valid() const noexcept;
        ~FormatHandle();

      public:
        FormatHandle(const FormatHandle&) noexcept = delete;
        FormatHandle operator=(const FormatHandle&) noexcept = delete;

        FormatHandle(FormatHandle&&) noexcept;
        FormatHandle& operator=(FormatHandle&&) noexcept;
    };
} // namespace FFmpeg
