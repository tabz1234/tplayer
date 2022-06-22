#include "Device.hpp"

extern "C" {
#include <AL/al.h>
#include <AL/alc.h>
}

#include <cstdio>

OpenAL::Device::Device() noexcept : alc_device_{alcOpenDevice(nullptr)}, alc_ctx_{alcCreateContext(alc_device_, nullptr)}
{
    if (alc_device_ == nullptr) [[unlikely]] {
        fprintf(stderr, "alcOpenDevice failed");
        return;
    }

    if (alc_ctx_ == nullptr) [[unlikely]] {
        fprintf(stderr, "alcCreateContext failed");
        return;
    }

    const auto c_api_ret = alcMakeContextCurrent(alc_ctx_);
    if (c_api_ret == 0) [[unlikely]] {
        fprintf(stderr, "alcMakeContextCurrent failed");
        return;
    }

    valid_ = true;
}

bool OpenAL::Device::valid() const noexcept
{
    return valid_;
}

OpenAL::Device::~Device()
{

    auto c_api_ret = alcMakeContextCurrent(nullptr);
    if (c_api_ret == 0) [[unlikely]] {
        fprintf(stderr, "alcMakeContextCurrent failed in destructor");
    }

    alcDestroyContext(alc_ctx_);

    c_api_ret = alcCloseDevice(alc_device_);
    if (c_api_ret == 0) [[unlikely]] {
        fprintf(stderr, "alcCloseDevice failed");
    }
}
