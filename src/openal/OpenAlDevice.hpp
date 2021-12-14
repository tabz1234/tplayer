#ifndef OPENALSOUNDDEVICE_HPP
#define OPENALSOUNDDEVICE_HPP

extern "C"
{
#include <AL/alc.h>
}

namespace openal {

class Device
{
    Device();
    ~Device();

    ALCdevice* alc_device_;
    ALCcontext* alc_ctx_;

  public:
    static Device& get_singleton();
};

}

#endif
