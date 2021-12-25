#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/rational.h>
}

#include <filesystem>
#include <list>
#include <optional>

#include "../check.hpp"
#include "Frame.hpp"

namespace FFmpeg {

using namespace std::string_literals;

template<MediaType Type_>
class MediaLoader final
{

    std::list<Frame<Type_>> frame_buffer_;

    AVRational av_time_base_;

    std::optional<int> stream_index_;

    AVCodecContext* av_codec_ctx_;

    AVFormatContext* av_format_ctx_;

    AVPacket* av_packet_;

  public:
    MediaLoader(const std::filesystem::path& filepath)
    {

        int c_api_ret;
        const auto loader_type_str = MediaTypeStrings.at(Type_).data() + " loader : "s;

        av_format_ctx_ = avformat_alloc_context();
        check(av_format_ctx_ != nullptr, loader_type_str + "avformat_alloc_context failed"s);

        c_api_ret = avformat_open_input(&av_format_ctx_, filepath.c_str(), nullptr, nullptr);

        check(c_api_ret == 0, loader_type_str + "avformat_open_input failed"s);

        AVCodecParameters* av_audio_codec_params = nullptr;
        AVCodec* av_audio_codec = nullptr;

        for (int i = 0, end = av_format_ctx_->nb_streams; i < end; ++i) {

            if (av_format_ctx_->streams[i]->codecpar->codec_type == (AVMediaType)Type_) {

                av_audio_codec_params = av_format_ctx_->streams[i]->codecpar;

                stream_index_ = i;

                av_time_base_ = av_format_ctx_->streams[i]->time_base;

                av_audio_codec = avcodec_find_decoder(av_audio_codec_params->codec_id);

                check(av_audio_codec != nullptr,
                      loader_type_str + "avcodec_find_decoder failed, codec id :"s +
                        std::to_string(av_audio_codec_params->codec_id));
            }
        }

        check(stream_index_.has_value(), loader_type_str + "failed to find stream"s);

        av_codec_ctx_ = avcodec_alloc_context3(av_audio_codec);
        check(av_codec_ctx_ != nullptr, loader_type_str + "avcodec_alloc_context3 failed"s);

        c_api_ret = avcodec_parameters_to_context(av_codec_ctx_, av_audio_codec_params);
        check(c_api_ret >= 0, loader_type_str + "avcodec_parameters_to_context failed"s);

        c_api_ret = avcodec_open2(av_codec_ctx_, av_audio_codec, nullptr);
        check(c_api_ret >= 0, loader_type_str + "avcodec_open2 failed"s);

        av_packet_ = av_packet_alloc();
        check(av_packet_ != nullptr, loader_type_str + "av_frame_alloc failed");
    }

    auto&& pop_buffer() noexcept { return std::move(frame_buffer_); }

    auto get_time_ratio() const noexcept { return av_time_base_; }

    bool decode_next_packet()
    {

        const auto loader_type_str = MediaTypeStrings.at(Type_).data() + " loader : "s;

        while (av_read_frame(av_format_ctx_, av_packet_) == 0) {

            if (av_packet_->stream_index == stream_index_) {

                int c_api_ret = avcodec_send_packet(av_codec_ctx_, av_packet_);
                if (c_api_ret != 0) {

                    check(c_api_ret == AVERROR(EAGAIN), loader_type_str + "avcodec_send_packet failed");

                    Frame<Type_> bloat_frame;
                    while (avcodec_receive_frame(av_codec_ctx_, bloat_frame.ptr()) != AVERROR_EOF) {
                    }

                    av_packet_unref(av_packet_);
                    continue;
                }

                do {

                    Frame<Type_> cur_frame;

                    c_api_ret = avcodec_receive_frame(av_codec_ctx_, cur_frame.ptr());
                    if (c_api_ret != 0)
                        break;

                    frame_buffer_.emplace_back(std::move(cur_frame));

                } while (1);

                av_packet_unref(av_packet_);
                return true;
            }
        }

        av_packet_unref(av_packet_);
        return false;
    }

    ~MediaLoader()
    {

        avformat_close_input(&av_format_ctx_);
        avformat_free_context(av_format_ctx_);

        av_packet_free(&av_packet_);

        avcodec_free_context(&av_codec_ctx_);
    }

  public:
    MediaLoader(const MediaLoader&) = delete;
    MediaLoader& operator=(const MediaLoader&) = delete;
};
}
