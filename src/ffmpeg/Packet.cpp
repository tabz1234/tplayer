#include "Packet.hpp"

#include <utility>

namespace FFmpeg {
    Packet::Packet() noexcept : handle{av_packet_alloc()}
    {
        if (handle == nullptr) [[unlikely]] {
            valid_ = false;
        }
    }

    bool Packet::valid() const noexcept
    {
        return valid_;
    }

    void Packet::wipe() noexcept
    {
        av_packet_unref(handle);
    }

    Packet::~Packet()
    {
        av_packet_free(&handle);
    }

    Packet::Packet(Packet&& rval) noexcept : handle{std::exchange(rval.handle, nullptr)}
    {
    }

    Packet& Packet::operator=(Packet&& rval) noexcept
    {
        std::swap(handle, rval.handle);
        return *this;
    }

} // namespace FFmpeg

