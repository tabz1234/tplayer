#include "decode_multi_packet.hpp"

namespace FFmpeg {
    int decode_multi_frame(const Codec& codec, const Packet& in, std::back_insert_iterator<std::vector<Frame>> out_it) noexcept
    {
        int ret;

        ret = avcodec_send_packet(codec.handle, in.handle);
        if (ret == AVERROR(EAGAIN)) [[unlikely]] {
            while (ret != AVERROR_EOF) {
                Frame bloat_frame;
                ret = avcodec_receive_frame(codec.handle, bloat_frame.handle);
            }
            decode_multi_frame(codec, in, out_it);
        }

        while (ret == 0) {
            Frame temp_frame;
            ret = avcodec_receive_frame(codec.handle, temp_frame.handle);
            out_it = std::move(temp_frame);
        }

        return ret;
    }
} // namespace FFmpeg
