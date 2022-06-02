#include "decode_video_packet.hpp"

namespace FFmpeg {
    int decode_video_packet(const Codec& codec, const Packet& in, Frame& out) noexcept
    {
        int ret;

        ret = avcodec_send_packet(codec.handle, in.handle);
        if (ret == AVERROR(EAGAIN)) [[unlikely]] {
            while (ret != AVERROR_EOF) {
                ret = avcodec_receive_frame(codec.handle, out.handle);
            }
            decode_video_packet(codec, in, out);
        }

        ret = avcodec_receive_frame(codec.handle, out.handle);

        return ret;
    }
} // namespace FFmpeg
