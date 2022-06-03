#include "HWAccelCodec.hpp"

#include <utility>

static enum AVPixelFormat hw_pix_fmt;
static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts);

namespace FFmpeg {

    HWAccelCodec::HWAccelCodec(const AVCodecParameters* const codec_params, const enum AVHWDeviceType dev_type) noexcept
    {
        const AVCodec* avcodec = avcodec_find_decoder(codec_params->codec_id);
        if (avcodec == nullptr) [[unlikely]] {
            valid_ = false;
            return;
        }

        for (int i = 0;; i++) {
            const AVCodecHWConfig* config = avcodec_get_hw_config(avcodec, i);

            if (config == nullptr) [[unlikely]] {
                valid_ = false;
                return;
            }

            if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == dev_type) {
                hw_pix_fmt = config->pix_fmt;
                break;
            }
        }

        handle = avcodec_alloc_context3(avcodec);
        if (handle == nullptr) [[unlikely]] {
            valid_ = false;
            return;
        }

        auto c_api_ret = avcodec_parameters_to_context(handle, codec_params);
        if (c_api_ret != 0) [[unlikely]] {
            valid_ = false;
            return;
        }

        handle->get_format = get_hw_format;

        c_api_ret = av_hwdevice_ctx_create(&av_buffer_ref_, dev_type, nullptr, nullptr, 0);
        if (c_api_ret != 0) {
            valid_ = false;
            return;
        }

        handle->hw_device_ctx = av_buffer_ref(av_buffer_ref_);

        c_api_ret = avcodec_open2(handle, avcodec, nullptr);
        if (c_api_ret != 0) [[unlikely]] {
            valid_ = false;
            return;
        }
    }

    bool HWAccelCodec::valid() const noexcept
    {
        return valid_;
    }

    HWAccelCodec::~HWAccelCodec()
    {
        avcodec_close(handle);
        avcodec_free_context(&handle);

        av_buffer_unref(&av_buffer_ref_);
    }

    HWAccelCodec::HWAccelCodec(HWAccelCodec&& rval) noexcept : handle{std::exchange(rval.handle, nullptr)}
    {
    }
    HWAccelCodec& HWAccelCodec::operator=(HWAccelCodec&& rval) noexcept
    {
        std::swap(rval.handle, handle);
        return *this;
    }
} // namespace FFmpeg

static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts)
{
    const enum AVPixelFormat* p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt) return *p;
    }

    return AV_PIX_FMT_NONE;
}
