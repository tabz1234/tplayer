#include "Buffer.hpp"

#include <optional>
#include <utility>

#include "../check.hpp"

using namespace std::string_literals;
using namespace OpenAl;

constexpr ALenum AL_FORMAT_MONO_FLOAT32 = 0x10010;
constexpr ALenum AL_FORMAT_STEREO_FLOAT32 = 0x10011;

Buffer::Buffer(FFmpeg::Frame<FFmpeg::MediaType::audio>&& audio_frame)
    : duration_{audio_frame.ptr()->pkt_duration}
{

    std::optional<ALenum> al_format;

    if (audio_frame.ptr()->channels == 1) {
        al_format = AL_FORMAT_MONO_FLOAT32;
    }
    else if (audio_frame.ptr()->channels == 2) {
        al_format = AL_FORMAT_STEREO_FLOAT32;
    }

    check(al_format.has_value(),
          "cannot find OpenAL sound format, channels count :" +
              std::to_string(audio_frame.ptr()->channels));

    alGenBuffers(1, &al_buffer_);

    alBufferData(al_buffer_,
                 al_format.value(),
                 audio_frame.ptr()->data[0],
                 audio_frame.ptr()->linesize[0],
                 audio_frame.ptr()->sample_rate);
}
int64_t Buffer::get_duration() const noexcept
{
    return duration_;
}

Buffer::Buffer(Buffer&& rval) noexcept
    : al_buffer_{std::exchange(rval.al_buffer_, 0)}, duration_{rval.duration_}
{
}

Buffer& Buffer::operator=(Buffer&& rval) noexcept
{
    std::swap(al_buffer_, rval.al_buffer_);
    duration_ = rval.duration_;

    return *this;
}
ALuint Buffer::get_al_buffer_id() const noexcept
{
    return al_buffer_;
}
ALuint* Buffer::get_al_buffer_ptr() noexcept
{
    return &al_buffer_;
}
const ALuint* Buffer::get_al_buffer_ptr() const noexcept
{
    return &al_buffer_;
}
Buffer::~Buffer()
{
    alDeleteBuffers(1, &al_buffer_);
}
