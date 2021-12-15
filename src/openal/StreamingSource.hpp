#ifndef OPENAL_STREAMING_SOURCE_HPP
#define OPENAL_STREAMING_SOURCE_HPP

extern "C"
{
#include <AL/al.h>
}

#include <list>

#include "ffmpeg/Frame.hpp"
#include "openal/Buffer.hpp"

namespace openal {

class StreamingSource
{
    ALuint al_source_;

    static constexpr float p_Pitch = 1.f;
    static constexpr float p_Gain = 1.f;
    static constexpr float p_Position[3] = { 0, 0, 0 };
    static constexpr float p_Velocity[3] = { 0, 0, 0 };

    std::list<openal::Buffer> buffer_list_;

  public:
    StreamingSource();

    void add_buffer(openal::Buffer&&);

    void play(const AVRational audio_ratio);

    ~StreamingSource();
};

} // namespace openal

#endif
