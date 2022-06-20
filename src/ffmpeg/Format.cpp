#include "Format.hpp"

namespace FFmpeg {
    Format::Format(const std::string_view fname) noexcept
    {
        if (!format_.valid()) [[unlikely]] {
            valid_ = false;
            return;
        }

        const auto c_api_ret = avformat_open_input(&format_.handle, fname.data(), nullptr, nullptr);
        if (c_api_ret != 0) [[unlikely]] {
            valid_ = false;
            fprintf(stderr, "avformat_open_input failed\n");
            return;
        }
    }

    AVFormatContext* Format::get() const noexcept
    {
        return format_.handle;
    }

    bool Format::valid() const noexcept
    {
        return valid_;
    }

    Format::~Format()
    {
        avformat_close_input(&format_.handle);
    }

} // namespace FFmpeg
