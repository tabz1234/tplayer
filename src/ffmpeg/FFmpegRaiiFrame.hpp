#ifndef FFMPEG_RAII_FRAME_HPP
#define FFMPEG_RAII_FRAME_HPP

extern "C"
{
#include <libavutil/frame.h>
}

#include <stdexcept>
#include <utility>

namespace ffmpeg {

template<AVMediaType>
class RaiiFrame final
{
  protected:
    AVFrame* av_frame_ = nullptr;

  public:
    RaiiFrame()
    {
        av_frame_ = av_frame_alloc();
        if (av_frame_ == nullptr) [[unlikely]] {
            throw std::runtime_error("av_frame_alloc failed");
        }
    }

    AVFrame* ptr() noexcept { return av_frame_; }

    ~RaiiFrame() { av_frame_free(&av_frame_); }

    RaiiFrame(RaiiFrame&& rval) noexcept { *this = std::move(rval); }
    RaiiFrame& operator=(RaiiFrame&& rval) noexcept
    {
        this->av_frame_ = std::exchange(rval.av_frame_, nullptr);
        return *this;
    }

  private:
    RaiiFrame(const RaiiFrame&) = delete;
    RaiiFrame& operator=(const RaiiFrame&) = delete;
};

} // namespace ffmpeg

#endif
