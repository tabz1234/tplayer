#include "scale_video_frame.hpp"

namespace FFmpeg {
    int scale_video_frame(SwsContext* scaler, const Frame& in, Frame& out) noexcept
    {
        int ret;
        ret = av_frame_copy_props(out.handle, in.handle);
        ret = av_frame_get_buffer(out.handle, 0);
        sws_scale(scaler, in.handle->data, in.handle->linesize, 0, in.handle->height, out.handle->data, out.handle->linesize);

        return ret;
    }
} // namespace FFmpeg
