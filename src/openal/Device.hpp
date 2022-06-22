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
    };

} // namespace OpenAL
