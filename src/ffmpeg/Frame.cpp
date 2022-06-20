#include "Frame.hpp"

#include <utility>

namespace FFmpeg {
    Frame::Frame() noexcept : handle{av_frame_alloc()}
    {
        if (handle == nullptr) [[unlikely]] {
            fprintf(stderr, "av_frame_alloc failed\n");
        }
    }

    bool Frame::valid() const noexcept
    {
        return handle != nullptr;
    }

    void Frame::wipe() noexcept
    {
        av_frame_unref(handle);
    }

    Frame::~Frame()
    {
        av_frame_free(&handle);
    }

    Frame::Frame(Frame&& rval) noexcept : handle{std::exchange(rval.handle, nullptr)}
    {
    }

    Frame& Frame::operator=(Frame&& rval) noexcept
    {
        std::swap(handle, rval.handle);
        return *this;
    }
} // namespace FFmpeg
