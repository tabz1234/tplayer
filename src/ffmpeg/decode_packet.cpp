#include "decode_packet.hpp"

namespace FFmpeg {
    int decode_packet(const Codec& codec, const Packet& in, Frame& out) noexcept
    {
        int ret;

        ret = avcodec_send_packet(codec.handle, in.handle);
        if (ret == AVERROR(EAGAIN)) [[unlikely]] {
            while (ret != AVERROR_EOF) {
                ret = avcodec_receive_frame(codec.handle, out.handle);
            }
            decode_packet(codec, in, out);
        }

        ret = avcodec_receive_frame(codec.handle, out.handle);

        return ret;
    }
    int decode_packet(const HWAccelCodec& codec, const Packet& in, Frame& temp, Frame& out) noexcept
    {
        int ret;

        ret = avcodec_send_packet(codec.handle, in.handle);
        if (ret == AVERROR(EAGAIN)) [[unlikely]] {
            while (ret != AVERROR_EOF) {
                ret = avcodec_receive_frame(codec.handle, temp.handle);
            }
            decode_packet(codec, in, temp, temp);
        }

        ret = avcodec_receive_frame(codec.handle, out.handle);
        if (ret != 0) return ret;

        ret = av_hwframe_transfer_data(out.handle, temp.handle, 0);

        return ret;
    }
} // namespace FFmpeg
