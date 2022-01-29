#include "StreamingSource.hpp"

extern "C" {
#include <AL/al.h>
}

#include <stdexcept>
#include <thread>

using namespace OpenAl;

StreamingSource::~StreamingSource()
{
    alDeleteSources(1, &al_source_);
}
StreamingSource::StreamingSource() noexcept
{
    alGenSources(1, &al_source_);
    alSourcef(al_source_, AL_PITCH, pitch_);
    alSourcef(al_source_, AL_GAIN, gain_);
    alSource3f(al_source_, AL_POSITION, position_[0], position_[1], position_[2]);
    alSource3f(al_source_, AL_VELOCITY, velocity_[0], velocity_[1], velocity_[2]);
}

void StreamingSource::queue_buffer(const ALuint buffer_id) noexcept
{
    alSourceQueueBuffers(al_source_, 1, &buffer_id);
}
void StreamingSource::unqueue_buffer(ALuint buffer_id) noexcept
{
    alSourceUnqueueBuffers(al_source_, 1, &buffer_id);
}

ALint StreamingSource::get_processed_buffers_count() const noexcept
{
    ALint processed_buffers_count;
    alGetSourcei(al_source_, AL_BUFFERS_PROCESSED, &processed_buffers_count);

    return processed_buffers_count;
}
void StreamingSource::play() noexcept
{
    alSourcePlay(al_source_);
}
void StreamingSource::pause() noexcept
{
    alSourcePause(al_source_);
}
ALenum StreamingSource::get_state() const noexcept
{
    ALenum state;
    alGetSourcei(al_source_, AL_SOURCE_STATE, &state);

    return state;
}
