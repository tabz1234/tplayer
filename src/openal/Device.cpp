#include "Device.hpp"

extern "C" {
#include <AL/al.h>
}

#include <stdexcept>

#include "../check.hpp"

using namespace OpenAl;

Device& Device::get_singleton() {

    static Device singleton_instance;
    return singleton_instance;
}

Device::Device()
    : alc_device_{alcOpenDevice(nullptr)}, alc_ctx_{alcCreateContext(alc_device_, nullptr)} {

    check(alc_device_ != nullptr, "alcOpenDevice failed");

    check(alc_ctx_ != nullptr, "alcCreateContext failed");

    const auto c_api_ret = alcMakeContextCurrent(alc_ctx_);
    check(c_api_ret != 0, "alcMakeContextCurrent failed");
}

Device::~Device() {

    auto c_api_ret = alcMakeContextCurrent(nullptr);
    check(c_api_ret != 0, "alcMakeContextCurrent failed in destructor");

    alcDestroyContext(alc_ctx_);

    c_api_ret = alcCloseDevice(alc_device_);
    check(c_api_ret != 0, "alcCloseDevice failed in destructor");
}
