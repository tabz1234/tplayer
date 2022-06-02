#pragma once

#include "Format.hpp"
#include "Packet.hpp"

namespace FFmpeg {
    int read_packet(const Format& format_ctx, Packet& out);
} // namespace FFmpeg
