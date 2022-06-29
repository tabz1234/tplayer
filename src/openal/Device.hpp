#pragma once

struct ALCdevice;
struct ALCcontext;

namespace OpenAL {
    struct Device final {

        Device() noexcept;

        bool valid() const noexcept;

        ~Device();

      private:
        ALCdevice* alc_device_; // DO NOT CHANGE ORDER OF MEMBERS
        ALCcontext* alc_ctx_;

        bool valid_ = false;

        Device(const Device&) noexcept = delete;
        Device& operator=(const Device&) noexcept = delete;

        Device(Device&&) noexcept = delete;
        Device& operator=(Device&&) noexcept = delete;
    };

} // namespace OpenAL
