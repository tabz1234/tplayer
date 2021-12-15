#include "Device.hpp"

extern "C"
{
#include <AL/al.h>
}

#include <stdexcept>

using namespace openal;

Device&
Device::get_singleton()
{
    static Device singleton_instance;
    return singleton_instance;
}

Device::Device()
{
    alc_device_ = alcOpenDevice(nullptr);
    if (!alc_device_) {
        throw std::runtime_error("alcOpenDevice failed");
    }

    alc_ctx_ = alcCreateContext(alc_device_, nullptr);
    if (!alc_ctx_) {
        throw std::runtime_error("alcCreateContext failed");
    }

    if (!alcMakeContextCurrent(alc_ctx_)) {
        throw std::runtime_error("alcMakeContextCurrent failed");
    }
}

Device::~Device()
{
    if (!alcMakeContextCurrent(nullptr)) {
        throw std::runtime_error("alcMakeContextCurrent(nullptr) failed, in OpenAlDevice destructor");
    }

    alcDestroyContext(alc_ctx_);

    if (!alcCloseDevice(alc_device_)) {
        throw std::runtime_error("alcCloseDevice failed, in OpenAlDevice destructor");
    }
}
