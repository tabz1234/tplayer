#pragma once

extern "C" {
#include <libavutil/frame.h>
}

#include <stdexcept>
#include <utility>

#include "../check.hpp"
#include "MediaType.hpp"

namespace FFmpeg {

    template <MediaType V>
    struct Frame final {

        Frame(AVFrame* existing_frame) noexcept {
            av_frame_ = existing_frame;
        }

        Frame() {
            av_frame_ = av_frame_alloc();
            check(av_frame_ != nullptr, " av_frame_alloc failed");
        }

        AVFrame* ptr() noexcept {
            return av_frame_;
        }

        ~Frame() {
            av_frame_free(&av_frame_);
        }

        Frame(Frame&& rval) noexcept : av_frame_{std::exchange(rval.av_frame_, nullptr)} {
        }
        Frame& operator=(Frame&& rval) noexcept {
            std::swap(av_frame_, rval.av_frame_);
            return *this;
        }

      private:
        AVFrame* av_frame_ = nullptr;

      public:
        Frame(const Frame&) = delete;
        Frame& operator=(const Frame&) = delete;
    };

} // namespace FFmpeg
