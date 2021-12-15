#ifndef FFMPEG_FRAME_HPP
#define FFMPEG_FRAME_HPP

extern "C"
{
#include <libavutil/frame.h>
}

#include <stdexcept>
#include <utility>

namespace FFmpeg {

template<AVMediaType>
class Frame final
{
  protected:
    AVFrame* av_frame_ = nullptr;

  public:
    constexpr Frame(AVFrame* existing_frame) noexcept { av_frame_ = existing_frame; }

    Frame()
    {
        av_frame_ = av_frame_alloc();
        if (av_frame_ == nullptr) [[unlikely]] {
            throw std::runtime_error("av_frame_alloc failed");
        }
    }

    constexpr AVFrame* ptr() noexcept { return av_frame_; }

    ~Frame() { av_frame_free(&av_frame_); }

    Frame(Frame&& rval) noexcept { *this = std::move(rval); }
    Frame& operator=(Frame&& rval) noexcept
    {
        this->~Frame();
        this->av_frame_ = std::exchange(rval.av_frame_, nullptr);
        return *this;
    }

  private:
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
};

} // namespace ffmpeg

#endif
