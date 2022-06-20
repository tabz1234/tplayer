#pragma once

extern "C" {
#include <libswscale/swscale.h>
}

namespace FFmpeg {

    SwsContext* create_sws_scaler(int src_width,
                                  int src_height,
                                  enum AVPixelFormat src_format,
                                  int dst_width,
                                  int dst_height,
                                  enum AVPixelFormat dst_format,
                                  int flags) noexcept;

    void destroy_sws_scaler(SwsContext* handle) noexcept;

} // namespace FFmpeg
