#include "openal/OpenAlBuffer.hpp"

#include <utility>

using namespace std::string_literals;
using namespace openal;

constexpr ALenum AL_FORMAT_MONO_FLOAT32 = 0x10010; //@TODO check AL extension support
constexpr ALenum AL_FORMAT_STEREO_FLOAT32 = 0x10011;

Buffer::Buffer(ffmpeg::RaiiFrame<AVMEDIA_TYPE_AUDIO> av_frame)
  : time_stamp_{ av_frame.ptr()->pkt_dts }
{

    ALenum al_format = AL_NONE;

    if (av_frame.ptr()->channels == 1) {
        al_format = AL_FORMAT_MONO_FLOAT32;
    } else if (av_frame.ptr()->channels == 2) {
        al_format = AL_FORMAT_STEREO_FLOAT32;
    }

    if (al_format == AL_NONE) [[unlikely]] {
        throw std::runtime_error("cannot find OpenAL sound format, channels count :" +
                                 std::to_string(av_frame.ptr()->channels));
    }

    alGenBuffers(1, &al_buffer_);

    alBufferData(al_buffer_,
                 al_format,
                 av_frame.ptr()->data[0],
                 av_frame.ptr()->linesize[0],
                 av_frame.ptr()->sample_rate); //@TODO intensive openal err check

#if 0
    ALenum c_api_err = alGetError();
    if (c_api_err != AL_NO_ERROR) [[unlikely]] {
        throw std::runtime_error("OpenAL error in OpenAlBuffer constructor :"s + alGetString(c_api_err));
    }
#endif
}

Buffer::TimeStampT
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
    this->al_buffer_ = std::exchange(rval.al_buffer_, 0);
    this->time_stamp_ = rval.time_stamp_;

    return *this;
}

Buffer::AlBufferT
Buffer::get_al_buffer() noexcept
{
    return al_buffer_;
}

Buffer::AlBufferT*
Buffer::get_al_buffer_ptr() noexcept
{
    return &al_buffer_;
}
Buffer::~Buffer()
{
    alDeleteBuffers(1, &al_buffer_);
}

