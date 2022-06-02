#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace FFmpeg {
    struct Codec final {

        AVCodecContext* handle;

        Codec(const AVCodecParameters* const codec_params) noexcept;
        bool valid() const noexcept;
        ~Codec();

      private:
        bool valid_ = true;

      public:
        Codec(const Codec&) noexcept = delete;
        Codec& operator=(const Codec&) noexcept = delete;

        Codec(Codec&&) noexcept;
        Codec& operator=(Codec&&) noexcept;
    };
} // namespace FFmpeg
