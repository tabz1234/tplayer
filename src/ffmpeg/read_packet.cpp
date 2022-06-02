#include "read_packet.hpp"

namespace FFmpeg {
    int read_packet(const Format& format_ctx, Packet& out)
    {
        return av_read_frame(format_ctx.get(), out.handle);
    }
} // namespace FFmpeg
