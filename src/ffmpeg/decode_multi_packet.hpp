#pragma once

#include "Codec.hpp"
#include "Frame.hpp"
#include "Packet.hpp"

#include <iterator>
#include <vector>

namespace FFmpeg {
    int decode_multi_frame(const Codec& codec, const Packet& in, std::back_insert_iterator<std::vector<Frame>> out_it) noexcept;
} // namespace FFmpeg
