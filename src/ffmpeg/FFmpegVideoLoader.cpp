#include "FFmpegVideoLoader.hpp"

#include <stdexcept>

using namespace std::string_literals;

ffmpeg::MediaLoader::MediaLoader(const std::filesystem::path& video_path)
  : video_path_{ video_path }
{
    int c_api_ret;

    av_format_ctx_ = avformat_alloc_context();
    if (av_format_ctx_ == nullptr) {
        throw std::runtime_error("avformat_alloc_context failed");
    }

    c_api_ret = avformat_open_input(&av_format_ctx_, video_path_.c_str(), nullptr, nullptr);

    if (c_api_ret != 0) {
        throw std::runtime_error("avformat_open_input failed");
    }

    AVCodecParameters* av_video_codec_params = nullptr;
    AVCodecParameters* av_audio_codec_params = nullptr;

    AVCodec* av_video_codec = nullptr;
    AVCodec* av_audio_codec = nullptr;

    for (int i = 0, end = av_format_ctx_->nb_streams; i < end; ++i) {

        if (av_format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

            av_video_codec_params = av_format_ctx_->streams[i]->codecpar;

            video_stream_index_ = i;

            av_video_time_base_ = av_format_ctx_->streams[i]->time_base;

            av_video_codec = avcodec_find_decoder(av_video_codec_params->codec_id);

            if (av_video_codec == nullptr) {
                throw std::runtime_error("avcodec_find_decoder failed, video codec id :" +
                                         std::to_string(av_video_codec_params->codec_id));
            }

        } else if (av_format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {

            av_audio_codec_params = av_format_ctx_->streams[i]->codecpar;

            audio_stream_index_ = i;

            av_audio_time_base_ = av_format_ctx_->streams[i]->time_base;

            av_audio_codec = avcodec_find_decoder(av_audio_codec_params->codec_id);

            if (av_audio_codec == nullptr) {
                throw std::runtime_error("avcodec_find_decoder failed, audio codec id :" +
                                         std::to_string(av_audio_codec_params->codec_id));
            }
        }
    }

    if (!video_stream_index_.has_value()) {
        throw std::runtime_error("Failed to find video stream");
    }

    av_video_codec_ctx_ = avcodec_alloc_context3(av_video_codec);
    if (av_video_codec_ctx_ == nullptr) {
        throw std::runtime_error("video decoder :  avcodec_alloc_context3 failed");
    }

    c_api_ret = avcodec_parameters_to_context(av_video_codec_ctx_, av_video_codec_params);
    if (c_api_ret < 0) {
        throw std::runtime_error("video decoder : avcodec_parameters_to_context failed");
    }

    c_api_ret = avcodec_open2(av_video_codec_ctx_, av_video_codec, nullptr);
    if (c_api_ret < 0) {
        throw std::runtime_error("video decoder : avcodec_open2 failed");
    }

    if (audio_stream_index_.has_value()) {

        av_audio_codec_ctx_ = avcodec_alloc_context3(av_audio_codec);
        if (av_audio_codec_ctx_ == nullptr) {
            throw std::runtime_error("audio decoder : avcodec_alloc_context3 failed");
        }

        c_api_ret = avcodec_parameters_to_context(av_audio_codec_ctx_, av_audio_codec_params);
        if (c_api_ret < 0) {
            throw std::runtime_error("audio decoder : avcodec_parameters_to_context failed");
        }

        c_api_ret = avcodec_open2(av_audio_codec_ctx_, av_audio_codec, nullptr);
        if (c_api_ret < 0) {
            throw std::runtime_error("audio decoder : avcodec_open2 failed");
        }
    }

    av_packet_ = av_packet_alloc();
    if (!av_packet_) {
        throw std::runtime_error("av_frame_alloc failed");
    }
}

std::list<ffmpeg::RaiiFrame<AVMEDIA_TYPE_AUDIO>>&
ffmpeg::MediaLoader::get_audio_buffer() noexcept
{
    return audio_frame_buffer_;
}
std::list<ffmpeg::RaiiFrame<AVMEDIA_TYPE_VIDEO>>&
ffmpeg::MediaLoader::get_video_buffer() noexcept
{
    return video_frame_buffer_;
}
void
ffmpeg::MediaLoader::clear_audio_buffer()
{
    audio_frame_buffer_.clear();
}
void
ffmpeg::MediaLoader::clear_video_buffer()
{
    video_frame_buffer_.clear();
}
AVRational
ffmpeg::MediaLoader::get_audio_ratio()
{
    if (!av_audio_time_base_.has_value()) [[unlikely]] {
        throw std::runtime_error("av_audio_time_base_ has no value");
    }

    return av_audio_time_base_.value();
}
bool
ffmpeg::MediaLoader::decode_next_packet()
{
#define DRT 1
#if DRT
    static int DBG_ITER_VIDEO{};
    static int DBG_ITER_AUDIO{};
#endif

    while (av_read_frame(av_format_ctx_, av_packet_) == 0) {

        if (av_packet_->stream_index == video_stream_index_) {

            int c_api_ret = avcodec_send_packet(av_video_codec_ctx_, av_packet_);
            if (c_api_ret != 0) [[unlikely]] {

                if (c_api_ret != AVERROR(EAGAIN)) {
                    throw std::runtime_error("avcodec_send_packet failed");
                } else {
                    RaiiFrame<AVMEDIA_TYPE_VIDEO> bloat_frame;

                    while (avcodec_receive_frame(av_video_codec_ctx_, bloat_frame.ptr()) != AVERROR_EOF) {
                    }

                    av_packet_unref(av_packet_);
                    continue;
                }
            }

            do {

                RaiiFrame<AVMEDIA_TYPE_VIDEO> video_frame;
                c_api_ret = avcodec_receive_frame(av_video_codec_ctx_, video_frame.ptr());
                if (c_api_ret != 0)
                    break;

#if DRT
                DBG_ITER_VIDEO++;
                printf("\nVIDEO |%4d|, pts = |%lf|\n",
                       DBG_ITER_VIDEO,
                       video_frame.ptr()->pts * av_video_time_base_.num /
                         (double)av_video_time_base_.den);
#endif
                video_frame_buffer_.emplace_back(std::move(video_frame));

            } while (1);

            av_packet_unref(av_packet_);
            return true;

        } else if (audio_stream_index_.has_value() && av_packet_->stream_index == audio_stream_index_) {

            int c_api_ret = avcodec_send_packet(av_audio_codec_ctx_, av_packet_);
            if (c_api_ret != 0) [[unlikely]] {

                if (c_api_ret != AVERROR(EAGAIN)) [[unlikely]] {
                    throw std::runtime_error("avcodec_send_packet failed");
                } else {
                    RaiiFrame<AVMEDIA_TYPE_AUDIO> bloat_frame;
                    while (avcodec_receive_frame(av_audio_codec_ctx_, bloat_frame.ptr()) != AVERROR_EOF) {
                    }

                    av_packet_unref(av_packet_);
                    continue;
                }
            }

            do {

                RaiiFrame<AVMEDIA_TYPE_AUDIO> audio_frame;

                c_api_ret = avcodec_receive_frame(av_audio_codec_ctx_, audio_frame.ptr());
                if (c_api_ret != 0)
                    break;

#if DRT
                DBG_ITER_AUDIO++;
                printf("AUDIO |%4d|, pts = |%lf|, linesize = |%d|\n",
                       DBG_ITER_AUDIO,
                       audio_frame.ptr()->pts * av_audio_time_base_->num /
                         (double)av_audio_time_base_->den,
                       audio_frame.ptr()->linesize[0]);
#endif
                audio_frame_buffer_.emplace_back(std::move(audio_frame));

            } while (1);

            av_packet_unref(av_packet_);
            return true;
        }
    }

    return false;
}

ffmpeg::MediaLoader::~MediaLoader()
{

    avformat_close_input(&av_format_ctx_);
    avformat_free_context(av_format_ctx_);

    av_packet_free(&av_packet_);

    avcodec_free_context(&av_video_codec_ctx_);
    avcodec_free_context(&av_audio_codec_ctx_);
}
