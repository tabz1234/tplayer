#pragma once

extern "C" {
#include <libavutil/frame.h>
}

namespace FFmpeg {
    struct Frame final {

        AVFrame* handle;

        Frame() noexcept;

        bool valid() const noexcept;
        void wipe() noexcept;

        ~Frame();

      public:
        Frame(const Frame&) noexcept = delete;
        Frame& operator=(const Frame&) noexcept = delete;

        Frame(Frame&&) noexcept;
        Frame& operator=(Frame&&) noexcept;
    };
} // namespace FFmpeg
