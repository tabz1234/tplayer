#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace FFmpeg {
    struct HWAccelCodec final {

        AVCodecContext* handle;

        HWAccelCodec(const AVCodecParameters* const codec_params, const enum AVHWDeviceType dev_type) noexcept;
        bool valid() const noexcept;
        ~HWAccelCodec();

        enum class Err {

        };

      private:
        bool valid_ = true;
        AVBufferRef* av_buffer_ref_;

      public:
        HWAccelCodec(const HWAccelCodec&) noexcept = delete;
        HWAccelCodec& operator=(const HWAccelCodec&) noexcept = delete;

        HWAccelCodec(HWAccelCodec&&) noexcept;
        HWAccelCodec& operator=(HWAccelCodec&&) noexcept;
    };
} // namespace FFmpeg
