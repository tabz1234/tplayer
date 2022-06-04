#pragma once

#include "Codec.hpp"
#include "Frame.hpp"
#include "HWAccelCodec.hpp"
#include "Packet.hpp"

namespace FFmpeg {
    int decode_packet(const Codec& codec, const Packet& in, Frame& out) noexcept;
    int decode_packet(const HWAccelCodec& codec, const Packet& in, Frame& temp, Frame& out) noexcept;
} // namespace FFmpeg
