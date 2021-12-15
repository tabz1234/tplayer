#include "openal/StreamingSource.hpp"

extern "C"
{
#include <AL/al.h>
}

#include <stdexcept>
#include <thread>

using namespace openal;

StreamingSource::StreamingSource()
{
    alGenSources(1, &al_source_);
    alSourcef(al_source_, AL_PITCH, p_Pitch);
    alSourcef(al_source_, AL_GAIN, p_Gain);
    alSource3f(al_source_, AL_POSITION, p_Position[0], p_Position[1], p_Position[2]);
    alSource3f(al_source_, AL_VELOCITY, p_Velocity[0], p_Velocity[1], p_Velocity[2]);
}
void
StreamingSource::play(const AVRational audio_ratio)
{

    for (auto& buff : buffer_list_) {
        alSourceQueueBuffers(al_source_, 1, buff.get_al_buffer_ptr());
    }

    alSourcePlay(al_source_);

#if 1
    const auto& a = std::chrono::duration<double>(
      (buffer_list_.back().get_time_stamp() - buffer_list_.front().get_time_stamp()) * audio_ratio.num /
      (double)audio_ratio.den);
    std::this_thread::sleep_for(a);
#else

    ALint state = AL_PLAYING;
    while (state == AL_PLAYING) { //@TODO 100% cpu usage
        alGetSourcei(al_source_, AL_SOURCE_STATE, &state);
    }
#endif

    for (auto& buff : buffer_list_) {
        alSourceUnqueueBuffers(al_source_, 1, buff.get_al_buffer_ptr());
    }

    buffer_list_.clear();
}

void
StreamingSource::add_buffer(openal::Buffer&& buffer)
{
    buffer_list_.emplace_back(std::move(buffer));
}
StreamingSource::~StreamingSource()
{
    alDeleteSources(1, &al_source_);
}
