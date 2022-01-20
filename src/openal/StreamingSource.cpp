#include "StreamingSource.hpp"

extern "C" {
#include <AL/al.h>
}

#include <stdexcept>
#include <thread>

using namespace OpenAl;

StreamingSource::StreamingSource() {

    queued_buffers_.reserve(30);

    alGenSources(1, &al_source_);
    alSourcef(al_source_, AL_PITCH, pitch_);
    alSourcef(al_source_, AL_GAIN, gain_);
    alSource3f(al_source_, AL_POSITION, position_[0], position_[1], position_[2]);
    alSource3f(al_source_, AL_VELOCITY, velocity_[0], velocity_[1], velocity_[2]);
}
void StreamingSource::play(const AVRational audio_ratio) {

    for (auto& buff : queued_buffers_) {
        alSourceQueueBuffers(al_source_, 1, buff.get_al_buffer_ptr());
    }

    alSourcePlay(al_source_);

    const auto play_duration_sec = std::chrono::duration<long double>(
        (queued_buffers_.back().get_time_stamp() - queued_buffers_.front().get_time_stamp()) *
        audio_ratio.num / static_cast<long double>(audio_ratio.den));

#if 1
    std::this_thread::sleep_for(play_duration_sec);

    ALint state = AL_PLAYING;
    while (state == AL_PLAYING) { // play_duration_sec doesnt include the last frame DURATION
        alGetSourcei(al_source_, AL_SOURCE_STATE, &state);
    }
#endif

    for (auto& buff : queued_buffers_) {
        alSourceUnqueueBuffers(al_source_, 1, buff.get_al_buffer_ptr());
    }

    queued_buffers_.clear();
}

void StreamingSource::add_buffer(OpenAl::Buffer&& buffer) {
    queued_buffers_.emplace_back(std::move(buffer));
}
StreamingSource::~StreamingSource() {
    alDeleteSources(1, &al_source_);
}
