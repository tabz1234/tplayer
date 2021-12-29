#include "InputHandle.hpp"

#include "../check.hpp"

using namespace FFmpeg;

InputHandle::InputHandle(const std::filesystem::path& filepath)
    : av_format_ctx_{avformat_alloc_context()} {

    check(av_format_ctx_ != nullptr, "avformat_alloc_context failed");

    const auto c_api_ret = avformat_open_input(&av_format_ctx_, filepath.c_str(), nullptr, nullptr);
    check(c_api_ret == 0, "avformat_open_input failed");
}

AVFormatContext* InputHandle::get_ptr() noexcept {
    return av_format_ctx_;
}

InputHandle::~InputHandle() {

    avformat_close_input(&av_format_ctx_);
    avformat_free_context(av_format_ctx_);
}
