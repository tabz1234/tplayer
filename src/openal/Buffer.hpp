#ifndef OPENAL_BUFFER_HPP
#define OPENAL_BUFFER_HPP

extern "C"
{
#include <AL/al.h>
}

#include <chrono>
#include <cstdint>

#include "ffmpeg/Frame.hpp"

namespace openal {

class Buffer final
{
    using AlBufferT = ALuint;
    using TimeStampT = int64_t;

    AlBufferT al_buffer_;
    TimeStampT time_stamp_;

  public:
    Buffer(FFmpeg::Frame<AVMEDIA_TYPE_AUDIO> av_frame);

    AlBufferT get_al_buffer() noexcept;
    AlBufferT* get_al_buffer_ptr() noexcept;

    TimeStampT get_time_stamp() const noexcept;

    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;

    ~Buffer();

  private:
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
};

}

#endif
