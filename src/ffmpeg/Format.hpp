#pragma once

#include "FormatHandle.hpp"

#include <string_view>

namespace FFmpeg {
    struct Format final {

        Format(const std::string_view fname) noexcept;

        AVFormatContext* get() const noexcept;
        bool valid() const noexcept;

        ~Format();

      private:
        FormatHandle format_;
        bool valid_ = true;
    };
} // namespace FFmpeg
