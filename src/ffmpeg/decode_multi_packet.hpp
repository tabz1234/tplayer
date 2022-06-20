#pragma once

#include "Codec.hpp"
#include "Frame.hpp"
#include "Packet.hpp"

#include <vector>

namespace FFmpeg {
    int decode_multi_packet(const Codec& codec, const Packet& in, std::vector<Frame>& out) noexcept;
} // namespace FFmpeg
