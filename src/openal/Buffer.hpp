#pragma once

extern "C" {
#include <AL/al.h>
}

#include <cstdint>

#include "../ffmpeg/Frame.hpp"

namespace OpenAl {

    struct Buffer final {

        Buffer(FFmpeg::Frame<FFmpeg::MediaType::audio>&& audio_frame);

        ALuint get_al_buffer_id() const noexcept;

        const ALuint* get_al_buffer_ptr() const noexcept;
        ALuint* get_al_buffer_ptr() noexcept;

        int64_t get_duration() const noexcept;

        Buffer(Buffer&&) noexcept;
        Buffer& operator=(Buffer&&) noexcept;

        ~Buffer();

      private:
        ALuint al_buffer_;
        int64_t duration_;

      public:
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
    };

} // namespace OpenAl
