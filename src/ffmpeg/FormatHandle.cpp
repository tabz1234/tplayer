#include "FormatHandle.hpp"

#include <utility>

namespace FFmpeg {
    FormatHandle::FormatHandle() noexcept : handle{avformat_alloc_context()}
    {
        if (handle == nullptr) [[unlikely]] {
            fprintf(stderr, "avformat_alloc_context failed\n");
            return;
        }
    }

    bool FormatHandle::valid() const noexcept
    {
        return handle != nullptr;
    }

    FormatHandle::~FormatHandle()
    {
        avformat_free_context(handle);
    }

    FormatHandle::FormatHandle(FormatHandle&& rval) noexcept : handle{std::exchange(rval.handle, nullptr)}
    {
    }
    FormatHandle& FormatHandle::operator=(FormatHandle&& rval) noexcept
    {
        std::swap(handle, rval.handle);
        return *this;
    }
} // namespace FFmpeg
