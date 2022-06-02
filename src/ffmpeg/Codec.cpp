#include "Codec.hpp"

#include <utility>

namespace FFmpeg {

    Codec::Codec(const AVCodecParameters* const codec_params) noexcept
    {
        const AVCodec* avcodec = avcodec_find_decoder(codec_params->codec_id);
        if (avcodec == nullptr) [[unlikely]] {
            valid_ = false;
        }

        handle = avcodec_alloc_context3(avcodec);
        if (handle == nullptr) [[unlikely]] {
            valid_ = false;
        }

        auto c_api_ret = avcodec_parameters_to_context(handle, codec_params);
        if (c_api_ret != 0) [[unlikely]] {
            valid_ = false;
        }

        c_api_ret = avcodec_open2(handle, avcodec, nullptr);
        if (c_api_ret != 0) [[unlikely]] {
            valid_ = false;
        }
    }

    bool Codec::valid() const noexcept
    {
        return valid_;
    }

    Codec::~Codec()
    {
        avcodec_close(handle);
        avcodec_free_context(&handle);
    }

    Codec::Codec(Codec&& rval) noexcept : handle{std::exchange(rval.handle, nullptr)}
    {
    }
    Codec& Codec::operator=(Codec&& rval) noexcept
    {
        std::swap(rval.handle, handle);
        return *this;
    }
} // namespace FFmpeg
