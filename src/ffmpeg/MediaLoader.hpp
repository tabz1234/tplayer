#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/rational.h>
}

#include <memory>
#include <optional>
#include <vector>

#include "../check.hpp"
#include "Frame.hpp"

namespace FFmpeg {
    using namespace std::string_literals;

    template <MediaType Type_>
    class MediaLoader final {

        AVFormatContext* av_format_ctx_;

        AVRational av_time_base_;
        int stream_index_;

        AVCodecContext* av_codec_ctx_;
        AVPacket* av_packet_;

      private:
        constexpr static auto type_cstr = MediaType_to_sv(Type_).data();

      public:
        MediaLoader(const std::string_view filepath) {

            av_format_ctx_ = avformat_alloc_context();
            check(av_format_ctx_ != nullptr, type_cstr + " loader avformat_alloc_context failed"s);

            auto c_api_ret =
                avformat_open_input(&av_format_ctx_, filepath.data(), nullptr, nullptr);
            check(c_api_ret == 0, type_cstr + " loader avformat_open_input failed"s);

            AVCodecParameters* av_codec_params = nullptr;
            AVCodec* av_codec = nullptr;

            bool stream_found = false;
            for (int i = 0, end = av_format_ctx_->nb_streams; i < end; ++i) {

                if (av_format_ctx_->streams[i]->codecpar->codec_type ==
                    static_cast<enum AVMediaType>(Type_)) {

                    stream_found = true;

                    av_codec_params = av_format_ctx_->streams[i]->codecpar;

                    stream_index_ = i;
                    av_time_base_ = av_format_ctx_->streams[i]->time_base;

                    av_codec = avcodec_find_decoder(av_codec_params->codec_id);
                    check(av_codec != nullptr,
                          type_cstr + " loader avcodec_find_decoder failed, codec id :"s +
                              std::to_string(av_codec_params->codec_id));
                }
            }

            check(stream_found, type_cstr + " loader failed to find stream"s);

            av_codec_ctx_ = avcodec_alloc_context3(av_codec);
            check(av_codec_ctx_ != nullptr, type_cstr + " loader avcodec_alloc_context3 failed"s);

            c_api_ret = avcodec_parameters_to_context(av_codec_ctx_, av_codec_params);
            check(c_api_ret >= 0, type_cstr + " loader avcodec_parameters_to_context failed"s);

            c_api_ret = avcodec_open2(av_codec_ctx_, av_codec, nullptr);
            check(c_api_ret >= 0, type_cstr + " loader avcodec_open2 failed"s);

            av_packet_ = av_packet_alloc();
            check(av_packet_ != nullptr, type_cstr + " loader av_frame_alloc failed"s);
        }

        auto get_time_ratio() const noexcept {
            return av_time_base_;
        }

        std::optional<std::vector<Frame<Type_>>> decode_next_packet() {

            const std::unique_ptr<AVPacket, void (*)(AVPacket*)> av_packet_uptr{
                av_packet_,
                [](AVPacket* raw_ptr) {
                    av_packet_unref(raw_ptr);
                }};

            while (av_read_frame(av_format_ctx_, av_packet_uptr.get()) == 0) {

                if (av_packet_uptr.get()->stream_index == stream_index_) {

                    std::vector<Frame<Type_>> frame_buffer_;

                    int c_api_ret = avcodec_send_packet(av_codec_ctx_, av_packet_uptr.get());
                    check(c_api_ret == 0, type_cstr + " loader avcodec_send_packet failed"s);

                    do {

                        Frame<Type_> cur_frame;

                        c_api_ret = avcodec_receive_frame(av_codec_ctx_, cur_frame.ptr());
                        if (c_api_ret != 0) break;
                        else {
                            frame_buffer_.emplace_back(std::move(cur_frame));
                        }

                    } while (1);

                    return frame_buffer_;
                }
            }

            return std::nullopt;
        }

        ~MediaLoader() {

            avformat_close_input(&av_format_ctx_);
            avformat_free_context(av_format_ctx_);

            av_packet_free(&av_packet_);

            avcodec_free_context(&av_codec_ctx_);
        }

      public:
        MediaLoader(const MediaLoader&) = delete;
        MediaLoader& operator=(const MediaLoader&) = delete;

        MediaLoader(MediaLoader&&) noexcept = delete;
        MediaLoader& operator=(MediaLoader&&) noexcept = delete;
    };
} // namespace FFmpeg
