#pragma once

extern "C" {
#include <AL/al.h>
}

#include <chrono>

#include "../ffmpeg/Frame.hpp"

namespace OpenAl {

    struct StreamingSource final {

        StreamingSource() noexcept;

        void queue_buffer(const ALuint buffer_id) noexcept;
        void unqueue_buffer(ALuint buffer_id) noexcept;

        ALint get_processed_buffers_count() const noexcept;

        void play() noexcept;
        void pause() noexcept;

        ALenum get_state() const noexcept;

        ~StreamingSource();

      private:
        ALuint al_source_;

      private:
        static constexpr float pitch_ = 1.f;
        static constexpr float gain_ = 1.f;
        static constexpr float position_[3] = {0, 0, 0};
        static constexpr float velocity_[3] = {0, 0, 0};

      public:
        StreamingSource(const StreamingSource&) = delete;
        StreamingSource& operator=(const StreamingSource&) = delete;

        StreamingSource(StreamingSource&&) = delete;
        StreamingSource& operator=(StreamingSource&&) = delete;
    };

} // namespace OpenAl
