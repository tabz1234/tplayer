#include "SWScaler.hpp"

namespace FFmpeg {
    SwsContext* create_sws_scaler(int src_width,
                                  int src_height,
                                  enum AVPixelFormat src_format,
                                  int dst_width,
                                  int dst_height,
                                  enum AVPixelFormat dst_format,
                                  int flags) noexcept
    {
        SwsContext* ret =
            sws_getContext(src_width, src_height, src_format, dst_width, dst_height, dst_format, flags, nullptr, nullptr, nullptr);
        return ret;
    }

    void destroy_sws_scaler(SwsContext* handle) noexcept
    {
        sws_freeContext(handle);
    }
} // namespace FFmpeg

