#include "VideoScaler.hpp"

#include "../check.hpp"

using namespace FFmpeg;

VideoScaler::VideoScaler(const int old_width,
                         const int old_height,
                         const int old_format,
                         const int new_width,
                         const int new_height,
                         const int new_format)

    : sws_ctx_{sws_getContext(old_width,
                              old_height,
                              static_cast<enum AVPixelFormat>(old_format),
                              new_width,
                              new_height,
                              static_cast<enum AVPixelFormat>(new_format),
                              SWS_BICUBIC,
                              nullptr,
                              nullptr,
                              nullptr)},
      new_width_{new_width}, new_height_{new_height}, new_format_{new_format}
{

    check(sws_ctx_ != nullptr, "sws_getContext failed");
}

Frame<MediaType::video> VideoScaler::scale(const Frame<MediaType::video>& input_frame)
{

    Frame<MediaType::video> converted_frame;

    converted_frame.ptr()->width = new_width_;
    converted_frame.ptr()->height = new_height_;
    converted_frame.ptr()->format = new_format_;

    const auto c_api_ret = av_frame_get_buffer(converted_frame.ptr(), 0);
    check(c_api_ret == 0, "av_frame_get_buffer failed");

    sws_scale(sws_ctx_,
              input_frame.ptr()->data,
              input_frame.ptr()->linesize,
              0,
              input_frame.ptr()->height,
              converted_frame.ptr()->data,
              converted_frame.ptr()->linesize);

    return converted_frame;
}

VideoScaler::~VideoScaler()
{
    sws_freeContext(sws_ctx_);
}

