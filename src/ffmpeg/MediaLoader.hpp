#ifndef FFMPEG_MEDIA_LOADER_HPP
#define FFMPEG_MEDIA_LOADER_HPP

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#include <filesystem>
#include <list>
#include <optional>

#include "ffmpeg/Frame.hpp"

namespace FFmpeg {

class MediaLoader final
{

    AVFormatContext* av_format_ctx_ = nullptr;

    std::optional<int> video_stream_index_;
    std::optional<int> audio_stream_index_;

    AVCodecContext* av_video_codec_ctx_ = nullptr;
    AVCodecContext* av_audio_codec_ctx_ = nullptr;

    AVPacket* av_packet_;

  protected:
    const std::filesystem::path video_path_;

    AVRational av_video_time_base_;
    std::optional<AVRational> av_audio_time_base_;

    std::list<Frame<AVMEDIA_TYPE_VIDEO>> video_frame_buffer_;
    std::list<Frame<AVMEDIA_TYPE_AUDIO>> audio_frame_buffer_;

  public:
    MediaLoader(const std::filesystem::path& video_path);

    std::list<Frame<AVMEDIA_TYPE_AUDIO>>& get_audio_buffer() noexcept;
    std::list<Frame<AVMEDIA_TYPE_VIDEO>>& get_video_buffer() noexcept;

    void clear_audio_buffer();
    void clear_video_buffer();

    AVRational get_audio_ratio();

    bool decode_next_packet();

    ~MediaLoader();
};

} // namespace ffmpeg

#endif
