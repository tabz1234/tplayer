#include "decode_multi_packet.hpp"

namespace FFmpeg {
    int decode_multi_packet(const Codec& codec, const Packet& in, std::vector<Frame>& out) noexcept
    {
        int ret;

        ret = avcodec_send_packet(codec.handle, in.handle);
        if (ret == AVERROR(EAGAIN)) [[unlikely]] {
            Frame bloat_frame;
            while (ret != AVERROR_EOF) {
                ret = avcodec_receive_frame(codec.handle, bloat_frame.handle);
                bloat_frame.wipe();
            }
            decode_multi_packet(codec, in, out);
        }

        auto it = out.begin();
        while (ret == 0) {
            if (it != out.end()) [[likely]] {
                ret = avcodec_receive_frame(codec.handle, it->handle);
                ++it;
            }
            else {
                Frame temp_frame;
                ret = avcodec_receive_frame(codec.handle, temp_frame.handle);
                out.emplace_back(std::move(temp_frame));
            }
        }

        return ret;
    }
} // namespace FFmpeg
