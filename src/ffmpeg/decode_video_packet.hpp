#pragma once

#include "Codec.hpp"
#include "Frame.hpp"
#include "Packet.hpp"

namespace FFmpeg {
    int decode_video_packet(const Codec& codec, const Packet& in, Frame& out) noexcept;
}
