#pragma once

extern "C" {
#include <libavcodec/packet.h>
}

namespace FFmpeg {
    struct Packet final {

        AVPacket* handle;

        Packet() noexcept;

        bool valid() const noexcept;
        void wipe() noexcept;

        ~Packet();

      private:
        bool valid_ = true;

      public:
        Packet(const Packet&) noexcept = delete;
        Packet& operator=(const Packet&) noexcept = delete;

        Packet(Packet&&) noexcept;
        Packet& operator=(Packet&&) noexcept;
    };

} // namespace FFmpeg
