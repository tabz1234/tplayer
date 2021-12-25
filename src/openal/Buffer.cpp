#include "openal/Buffer.hpp"

#include <utility>

#include "../check.hpp"

using namespace std::string_literals;
using namespace OpenAl;

constexpr ALenum AL_FORMAT_MONO_FLOAT32 = 0x10010; //@TODO check AL extension support
constexpr ALenum AL_FORMAT_STEREO_FLOAT32 = 0x10011;

Buffer::Buffer(FFmpeg::Frame<FFmpeg::MediaType::audio>&& audio_frame)
  : time_stamp_{ audio_frame.ptr()->pkt_dts }
{

    ALenum al_format = AL_NONE;

    if (audio_frame.ptr()->channels == 1) {
        al_format = AL_FORMAT_MONO_FLOAT32;
    } else if (audio_frame.ptr()->channels == 2) {
        al_format = AL_FORMAT_STEREO_FLOAT32;
    }

    check(al_format != AL_NONE,
          "cannot find OpenAL sound format, channels count :" +
            std::to_string(audio_frame.ptr()->channels));

    alGenBuffers(1, &al_buffer_);

    alBufferData(al_buffer_,
                 al_format,
                 audio_frame.ptr()->data[0],
                 audio_frame.ptr()->linesize[0],
                 audio_frame.ptr()->sample_rate);
}
int64_t
Buffer::get_time_stamp() const noexcept
{
    return time_stamp_;
}

Buffer::Buffer(Buffer&& rval) noexcept
{
    *this = std::move(rval);
}

Buffer&
Buffer::operator=(Buffer&& rval) noexcept
{
    // this->~Buffer();//leads to incorrect playback, i dont know how openal manages resources, but
    // without destructor three are no memory leaks or any other problems

    this->al_buffer_ = std::exchange(rval.al_buffer_, 0);
    this->time_stamp_ = rval.time_stamp_;

    return *this;
}
ALuint
Buffer::get_al_buffer() noexcept
{
    return al_buffer_;
}
ALuint*
Buffer::get_al_buffer_ptr() noexcept
{
    return &al_buffer_;
}
Buffer::~Buffer()
{
    alDeleteBuffers(1, &al_buffer_);
}

