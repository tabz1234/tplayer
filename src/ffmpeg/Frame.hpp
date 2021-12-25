#pragma once

extern "C"
{
#include <libavutil/frame.h>
}

#include <stdexcept>
#include <utility>

#include "../check.hpp"
#include "ffmpeg/MediaType.hpp"

namespace FFmpeg {

template<MediaType V>
struct Frame final
{
    const MediaType type = V;

    Frame(AVFrame* existing_frame) noexcept { av_frame_ = existing_frame; }

    Frame()
    {
        av_frame_ = av_frame_alloc();
        check(av_frame_ != nullptr, " av_frame_alloc failed");
    }

    AVFrame* ptr() noexcept { return av_frame_; }

    ~Frame() { av_frame_free(&av_frame_); }

    Frame(Frame&& rval) noexcept { *this = std::move(rval); }
    Frame& operator=(Frame&& rval) noexcept
    {
        this->~Frame();
        this->av_frame_ = std::exchange(rval.av_frame_, nullptr);
        return *this;
    }

  private:
    AVFrame* av_frame_ = nullptr;

  public:
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
};

} // namespace FFmpeg

