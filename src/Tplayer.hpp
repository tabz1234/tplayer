#pragma once

#include <filesystem>
#include <future>
#include <optional>
#include <queue>

#include "openal/Buffer.hpp"
#include "openal/Device.hpp"
#include "openal/StreamingSource.hpp"

#include "ffmpeg/MediaLoader.hpp"

#include "TerminalImage.hpp"

struct Tplayer final {

    Tplayer(const int argc, const char** const argv);

    void run();
    static void print_msg_prefix();

    ~Tplayer();

  private:
    void parse_cmd_arguments(const int argc, const char** const argv);

    /*audio group*/
    std::pair<std::future<void>, std::future<void>> async_start_audio_loops();

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::audio>> audio_loader_;
    std::atomic<bool> audio_play_done_;

    AVRational audio_time_ratio_;
    static constexpr auto AUDIO_BUFFER_SIZE_ = 20;

    std::condition_variable play_cond_;
    std::condition_variable cleanup_cond_;
    std::mutex play_cleanup_mutex_;

    std::queue<OpenAl::Buffer> audio_queue_;

    std::optional<OpenAl::StreamingSource> sound_source_;

    /*video group*/
    std::pair<std::future<void>, std::future<void>> async_start_video_loops();

    static constexpr auto VIDEO_BUFFER_SIZE_ = 8;
    AVRational video_time_ratio_;

    std::atomic<bool> video_consumer_done_;

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::video>> video_loader_;

    std::condition_variable video_producer_cond_;
    std::condition_variable video_consumer_cond_;
    std::mutex video_prod_cons_mutex_;

    std::queue<TerminalImage> video_queue_;
    std::atomic<bool> video_producer_done_;

  private:
    std::filesystem::path filepath_;
    bool has_video_;
    bool has_audio_;

    bool loop_ = false;

  private:
    Tplayer(const Tplayer&) = delete;
    Tplayer& operator=(const Tplayer&) = delete;

    Tplayer(Tplayer&&) noexcept = delete;
    Tplayer& operator=(Tplayer&&) noexcept = delete;
};
