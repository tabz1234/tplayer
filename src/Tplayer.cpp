#include "Tplayer.hpp"

#include <chrono>
#include <csignal>
#include <cstdlib>

#include "terminal/Terminal.hpp"

#include "ffmpeg/apply_time_ratio.hpp"
#include "ffmpeg/convert_audio_buffer.hpp"
#include "ffmpeg/scale_video_buffer.hpp"
#include "ffmpeg/shrink_audio_size_to_content.hpp"

#include "frame_to_esqmap.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;

Tplayer::Tplayer(const int argc, const char** const argv)
{
    parse_cmd_arguments(argc, argv);

    const auto print_disable_warning = [](const std::string_view type_sv,
                                          const std::string_view emsg_sv) {
        constexpr auto WARNING_COLOR = RGB_t{200, 180, 0};
        print_msg_prefix();
        Terminal::out(WARNING_COLOR,
                      type_sv.data() + " disabled "s,
                      Terminal::DefaultAttr,
                      "because : ",
                      emsg_sv.data(),
                      Terminal::newl);
    };

    try {
        audio_loader_.emplace(filepath_.c_str());
        audio_time_ratio_ = audio_loader_->get_time_ratio();

        OpenAl::Device::get_singleton();
        sound_source_.emplace();
    }
    catch (const std::exception& e) {
        print_disable_warning("audio", e.what());
    }

    try {
        video_loader_.emplace(filepath_.c_str());
        video_time_ratio_ = video_loader_->get_time_ratio();
    }
    catch (const std::exception& e) {
        print_disable_warning("video", e.what());
    }

    has_audio_ = audio_loader_.has_value();
    has_video_ = video_loader_.has_value();
    check(has_audio_ || has_video_, "input file contains neither video nor audio");

    video_consumer_done_ = has_video_ ? false : true;
    audio_play_done_ = has_audio_ ? false : true;

    Terminal::start_tui_mode();

    std::signal(SIGINT, [](int) {
    });
}
void Tplayer::run()
{
    std::optional<std::pair<std::future<void>, std::future<void>>> audio_loops_futures;
    if (has_audio_) {
        audio_loops_futures = async_start_audio_loops();
    }

    std::optional<std::pair<std::future<void>, std::future<void>>> video_loops_futures;
    if (has_video_) {
        video_loops_futures = async_start_video_loops();
    }

    bool exit_main = false;
    while (!exit_main) {
        if (has_video_ && video_consumer_done_) {
            exit_main = true;
        }
        if (has_audio_ && audio_play_done_) {
            exit_main = true;
        }

        char ch = '\0';
        read(STDIN_FILENO, &ch, 1);

        if (ch == 'q') {
            Terminal::stop_tui_mode();
            std::exit(EXIT_SUCCESS);
        }

        if (!exit_main) {
            std::this_thread::sleep_for(100ms);
        }
    }
}
std::pair<std::future<void>, std::future<void>> Tplayer::async_start_audio_loops()
{
    auto play_loop_fut = std::async(std::launch::async, [&] {
        while (!audio_play_done_) {
            std::vector<FFmpeg::Frame<FFmpeg::MediaType::audio>> raw_frames_vec;
            raw_frames_vec.reserve(AUDIO_BUFFER_SIZE_);

            for (int i = 0; i < AUDIO_BUFFER_SIZE_; ++i) {

                auto decoder_output = audio_loader_->decode_next_packet();
                if (decoder_output.has_value() != true) {
                    audio_play_done_ = true;
                    break;
                }
                for (auto&& raw_frame : decoder_output.value()) {
                    raw_frames_vec.emplace_back(std::move(raw_frame));
                }
            }

            std::vector<OpenAl::Buffer> al_buffer_vec;
            al_buffer_vec.reserve(AUDIO_BUFFER_SIZE_);

            if (raw_frames_vec.size() > 0) {

                auto interleaved_audio_buffer =
                    FFmpeg::convert_audio_buffer_format(raw_frames_vec.cbegin(),
                                                        raw_frames_vec.cend(),
                                                        AV_SAMPLE_FMT_FLT);

                for (auto& interleaved_frame : interleaved_audio_buffer) {
                    FFmpeg::shrink_audio_size_to_content(interleaved_frame.ptr());

                    auto&& al_buffer = OpenAl::Buffer(std::move(interleaved_frame));
                    sound_source_->queue_buffer(al_buffer.get_al_buffer_id());
                    al_buffer_vec.emplace_back(std::move(al_buffer));
                }
            }

            if (sound_source_->get_state() == AL_STOPPED ||
                sound_source_->get_state() == AL_INITIAL) {
                sound_source_->play();
            }

            { // mutex
                std::unique_lock<std::mutex> lk{play_cleanup_mutex_};

                play_cond_.wait(lk, [&] {
                    return audio_queue_.size() < AUDIO_BUFFER_SIZE_ || audio_play_done_;
                });

                for (auto&& al_buff : al_buffer_vec) {
                    audio_queue_.emplace(std::move(al_buff));
                }
            } // mutex
            cleanup_cond_.notify_one();
        }
        cleanup_cond_.notify_all();
    });

    auto cleanup_loop_fut = std::async(std::launch::async, [&] {
        while (!audio_play_done_) {

            std::vector<OpenAl::Buffer> al_buffer_vec;
            al_buffer_vec.reserve(AUDIO_BUFFER_SIZE_);

            { // mutex
                std::unique_lock<std::mutex> lk{play_cleanup_mutex_};

                cleanup_cond_.wait(lk, [&] {
                    return audio_queue_.size() > 0 || audio_play_done_;
                });

                while (audio_queue_.size() > 0) {
                    al_buffer_vec.emplace_back(std::move(audio_queue_.front()));
                    audio_queue_.pop();
                }
            } // mutex
            play_cond_.notify_one();

            std::chrono::duration<long double> duration_to_wait{0};
            for (const auto& al_buff : al_buffer_vec) {
                duration_to_wait +=
                    FFmpeg::apply_time_ratio(al_buff.get_duration(), audio_time_ratio_);
            }

            std::this_thread::sleep_for(duration_to_wait);

            for (const auto& al_buff : al_buffer_vec) {
                sound_source_->unqueue_buffer(al_buff.get_al_buffer_id());
            }
        }
    });

    return {std::move(play_loop_fut), std::move(cleanup_loop_fut)};
}
std::pair<std::future<void>, std::future<void>> Tplayer::async_start_video_loops()
{
    auto produce_loop = std::async(std::launch::async, [&] {
        while (!video_producer_done_) {

            std::vector<TerminalImage> final_images_vec;
            final_images_vec.reserve(VIDEO_BUFFER_SIZE_);

            std::vector<FFmpeg::Frame<FFmpeg::MediaType::video>> raw_frames_vec;
            raw_frames_vec.reserve(VIDEO_BUFFER_SIZE_);

            for (int i = 0; i < VIDEO_BUFFER_SIZE_; ++i) {
                auto decoder_output = video_loader_->decode_next_packet();
                if (decoder_output.has_value() != true) {
                    video_producer_done_ = true;
                    break;
                }

                for (auto&& raw_frame : decoder_output.value()) {
                    raw_frames_vec.emplace_back(std::move(raw_frame));
                }
            }

            if (raw_frames_vec.size() > 0) {

                Terminal::update_size();
                const auto rescaled_frames =
                    scale_video_buffer<decltype(raw_frames_vec)>(raw_frames_vec.cbegin(),
                                                                 raw_frames_vec.cend(),
                                                                 Terminal::get_size().ws_col,
                                                                 Terminal::get_size().ws_row);

                for (const auto& rescaled_frame : rescaled_frames) {
                    final_images_vec.emplace_back(frame_to_esqmap(rescaled_frame.ptr()),
                                                  rescaled_frame.ptr()->pkt_duration,
                                                  rescaled_frame.ptr()->pts);
                }
            }

            { // mutex
                std::unique_lock<std::mutex> lk{video_prod_cons_mutex_};

                video_producer_cond_.wait(lk, [&] {
                    return video_queue_.size() < VIDEO_BUFFER_SIZE_ || video_consumer_done_;
                });

                for (auto&& image : final_images_vec) {
                    video_queue_.emplace(std::move(image));
                }
            } // mutex
            video_consumer_cond_.notify_one();
        }
        video_consumer_cond_.notify_all();
    });
    auto consume_loop = std::async(std::launch::async, [&] {
        Terminal::suspend_stderr(); //@TODO raii wrapper

        const auto play_start_time = std::chrono::high_resolution_clock::now();
        while (!video_consumer_done_) {

            std::vector<TerminalImage> images_vec;
            images_vec.reserve(VIDEO_BUFFER_SIZE_);

            { // mutex
                std::unique_lock<std::mutex> lk{video_prod_cons_mutex_};

                if (video_producer_done_.load() && video_queue_.size() == 0) {
                    video_consumer_done_ = true;
                    break;
                }

                video_consumer_cond_.wait(lk, [&] {
                    return video_queue_.size() > 0 || video_consumer_done_;
                });

                while (video_queue_.size() > 0) {
                    images_vec.emplace_back(std::move(video_queue_.front()));
                    video_queue_.pop();
                }

            } // mutex
            video_producer_cond_.notify_one();

            for (const auto& image : images_vec) {

                const auto start_of_consuming = std::chrono::high_resolution_clock::now();
                const long double sleep_time =
                    (image.pts * video_time_ratio_.num /
                     static_cast<long double>(video_time_ratio_.den)) -
                    std::chrono::duration<long double>(start_of_consuming - play_start_time)
                        .count();

                std::this_thread::sleep_for(std::chrono::duration<long double>(sleep_time));

                write(STDOUT_FILENO, image.esqmap.data(), image.esqmap.size());
            }
        }

        Terminal::enable_stderr();
    });

    return {std::move(produce_loop), std::move(consume_loop)};
}
void Tplayer::parse_cmd_arguments(const int argc, const char** const argv)
{
    //@TODO argument parsing
    if (argc == 1) {
        throw std::runtime_error("no input file");
    }

    filepath_ = argv[1];

    const bool path_valid = std::filesystem::is_regular_file(filepath_);
    check(path_valid, "incorrect file path :" + filepath_.string());
}
void Tplayer::print_msg_prefix()
{
    constexpr auto brace_color = RGB_t{50, 200, 255};
    Terminal::out(brace_color, "[", Terminal::DefaultAttr, "tplayer", brace_color, "] ");
}
Tplayer::~Tplayer()
{
    Terminal::stop_tui_mode();
}
