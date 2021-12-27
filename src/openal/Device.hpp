#pragma once

extern "C" {
#include <AL/alc.h>
}

namespace OpenAl {

    struct Device {
        static Device& get_singleton();

      private:
        Device();
        ~Device();

        ALCdevice* alc_device_;
        ALCcontext* alc_ctx_;
    };

} // namespace OpenAl
