#pragma once

extern "C" {
#include <AL/al.h>
}

#include <list>

#include "ffmpeg/Frame.hpp"
#include "openal/Buffer.hpp"

namespace OpenAl {

    struct StreamingSource final {

        StreamingSource();

        void add_buffer(OpenAl::Buffer&&);

        void play(const AVRational audio_ratio);

        ~StreamingSource();

      private:
        ALuint al_source_;

        std::list<OpenAl::Buffer> buffer_list_;

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
