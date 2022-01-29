#include "Tplayer.hpp"

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <queue>
#include <thread>

#include "terminal/Terminal.hpp"

#include "openal/Buffer.hpp"
#include "openal/Device.hpp"
#include "openal/StreamingSource.hpp"

#include "ffmpeg/MediaLoader.hpp"
#include "ffmpeg/apply_time_ratio.hpp"
#include "ffmpeg/convert_audio_buffer.hpp"
#include "ffmpeg/scale_video_buffer.hpp"
#include "ffmpeg/shrink_audio_size_to_content.hpp"

#include "TerminalImage.hpp"
#include "frame_to_esqmap.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;

Tplayer::Tplayer(const int argc, const char** const argv)
{
    parse_cmd_arguments(argc, argv);

    std::signal(SIGINT, [](int) {
        Tplayer::print_msg_prefix();
        Terminal::out("interrupted by user, exiting ...", Terminal::newl);

        std::exit(EXIT_SUCCESS);
    });
}
void Tplayer::run()
{
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

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::video>> video_loader;
    try {
        video_loader.emplace(filepath_.c_str());

        Terminal::start_tui_mode();
        atexit([] {
            Terminal::stop_tui_mode();
        });
    }
    catch (const std::exception& e) {
        print_disable_warning("video", e.what());
    }

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::audio>> audio_loader;
    try {
        audio_loader.emplace(filepath_.c_str());
        OpenAl::Device::get_singleton();
    }
    catch (const std::exception& e) {
        print_disable_warning("audio", e.what());
    }

    bool has_audio = audio_loader.has_value();
    bool has_video = video_loader.has_value();
    check(has_audio || has_video, "input file contains neither video nor audio");

    std::exception_ptr audio_play_thread_eptr;
    std::exception_ptr audio_cleanup_thread_eptr;

    std::condition_variable play_cond;
    std::condition_variable cleanup_cond;
    std::mutex play_cleanup_mutex;

    std::queue<OpenAl::Buffer> audio_queue;
    OpenAl::StreamingSource sound_source;

    std::atomic<bool> audio_play_thread_done = false;

    const auto audio_time_ratio = audio_loader->get_time_ratio();
    constexpr auto AUDIO_BUFFER_SIZE = 50;

    if (has_audio) {
        std::jthread audio_play_thread([&] {
            try {
                while (!audio_play_thread_done) {
                    std::vector<FFmpeg::Frame<FFmpeg::MediaType::audio>> raw_frames_vec;
                    raw_frames_vec.reserve(AUDIO_BUFFER_SIZE);

                    for (int i = 0; i < AUDIO_BUFFER_SIZE; ++i) {

                        auto decoder_output = audio_loader->decode_next_packet();
                        if (decoder_output.has_value() != true) {
                            audio_play_thread_done = true;
                            break;
                        }
                        for (auto&& raw_frame : decoder_output.value()) {
                            raw_frames_vec.emplace_back(std::move(raw_frame));
                        }
                    }

                    std::vector<OpenAl::Buffer> al_buffer_vec;
                    al_buffer_vec.reserve(AUDIO_BUFFER_SIZE);

                    if (raw_frames_vec.size() > 0) {

                        auto interleaved_audio_buffer =
                            FFmpeg::convert_audio_buffer_format(raw_frames_vec.cbegin(),
                                                                raw_frames_vec.cend(),
                                                                AV_SAMPLE_FMT_FLT);

                        for (auto& interleaved_frame : interleaved_audio_buffer) {
                            FFmpeg::shrink_audio_size_to_content(interleaved_frame.ptr());

                            auto&& al_buffer = OpenAl::Buffer(std::move(interleaved_frame));
                            sound_source.queue_buffer(al_buffer.get_al_buffer_id());
                            al_buffer_vec.emplace_back(std::move(al_buffer));
                        }
                    }

                    if (sound_source.get_state() == AL_STOPPED ||
                        sound_source.get_state() == AL_INITIAL) {
                        sound_source.play();
                    }

                    { // mutex
                        std::unique_lock<std::mutex> lk{play_cleanup_mutex};

                        play_cond.wait(lk, [&] {
                            return audio_queue.size() <= AUDIO_BUFFER_SIZE;
                        });
                        for (auto&& al_buff : al_buffer_vec) {
                            audio_queue.emplace(std::move(al_buff));
                        }
                    } // mutex
                    cleanup_cond.notify_one();
                }
            }
            catch (const std::exception&) {
                audio_play_thread_eptr = std::current_exception();
            }
        });
        audio_play_thread.detach();

        std::jthread audio_cleanup_thread([&] {
            try {
                while (!audio_play_thread_done) {
                    std::vector<OpenAl::Buffer> al_buffer_vec;
                    al_buffer_vec.reserve(AUDIO_BUFFER_SIZE);

                    { // mutex
                        std::unique_lock<std::mutex> lk{play_cleanup_mutex};

                        if (audio_play_thread_done) {
                            play_cond.notify_one();
                            break;
                        };

                        cleanup_cond.wait(lk, [&] {
                            return audio_queue.size() > 0;
                        });

                        while (audio_queue.size() > 0) {
                            al_buffer_vec.emplace_back(std::move(audio_queue.front()));
                            audio_queue.pop();
                        }
                    } // mutex
                    play_cond.notify_one();

                    std::chrono::duration<long double> duration_to_wait{0};
                    for (const auto& al_buff : al_buffer_vec) {
                        duration_to_wait +=
                            FFmpeg::apply_time_ratio(al_buff.get_duration(), audio_time_ratio);
                    }

                    std::this_thread::sleep_for(duration_to_wait);

                    for (const auto& al_buff : al_buffer_vec) {
                        sound_source.unqueue_buffer(al_buff.get_al_buffer_id());
                    }
                }
            }
            catch (const std::exception&) {
                audio_cleanup_thread_eptr = std::current_exception();
            }
        });
        audio_cleanup_thread.detach();
    }

    std::exception_ptr video_producer_thread_eptr;
    std::exception_ptr video_consumer_thread_eptr;

    std::condition_variable video_producer_cond;
    std::condition_variable video_consumer_cond;
    std::mutex video_prod_cons_mutex;

    std::queue<TerminalImage> video_queue;
    std::atomic<bool> video_producer_done;

    constexpr auto VIDEO_BUFFER_SIZE = 8;
    const auto video_time_ratio = video_loader->get_time_ratio();

    std::atomic<bool> video_consumer_done = false;

    if (has_video) {
        std::jthread video_producer_thread([&] {
            try {
                while (!video_producer_done) {

                    std::vector<TerminalImage> final_images_vec;
                    final_images_vec.reserve(VIDEO_BUFFER_SIZE);

                    std::vector<FFmpeg::Frame<FFmpeg::MediaType::video>> raw_frames_vec;
                    raw_frames_vec.reserve(VIDEO_BUFFER_SIZE);

                    for (int i = 0; i < VIDEO_BUFFER_SIZE; ++i) {
                        auto decoder_output = video_loader->decode_next_packet();
                        if (decoder_output.has_value() != true) {
                            video_producer_done = true;
                            break;
                        }

                        for (auto&& raw_frame : decoder_output.value()) {
                            raw_frames_vec.emplace_back(std::move(raw_frame));
                        }
                    }

                    if (raw_frames_vec.size() > 0) {

                        Terminal::update_size();
                        const auto rescaled_frames = scale_video_buffer<decltype(raw_frames_vec)>(
                            raw_frames_vec.cbegin(),
                            raw_frames_vec.cend(),
                            Terminal::get_size().ws_col,
                            Terminal::get_size().ws_row);

                        for (const auto& rescaled_frame : rescaled_frames) {
                            final_images_vec.emplace_back(frame_to_esqmap(rescaled_frame.ptr()),
                                                          rescaled_frame.ptr()->pkt_duration,
                                                          rescaled_frame.ptr()->pkt_dts);
                        }
                    }

                    { // mutex
                        std::unique_lock<std::mutex> lk{video_prod_cons_mutex};

                        video_producer_cond.wait(lk, [&] {
                            return video_queue.size() <= VIDEO_BUFFER_SIZE;
                        });
                        for (auto&& image : final_images_vec) {
                            video_queue.emplace(std::move(image));
                        }
                    } // mutex
                    video_consumer_cond.notify_one();
                }
            }
            catch (const std::exception&) {
                video_producer_thread_eptr = std::current_exception();
            }
        });
        video_producer_thread.detach();

        std::jthread video_consumer_thread([&] {
            try {
                const auto play_start_time = std::chrono::high_resolution_clock::now();
                while (!video_consumer_done) {

                    std::vector<TerminalImage> images_vec;
                    images_vec.reserve(VIDEO_BUFFER_SIZE);

                    { // mutex
                        std::unique_lock<std::mutex> lk{video_prod_cons_mutex};

                        if (video_producer_done.load() && video_queue.size() == 0) {
                            video_consumer_done = true;
                            break;
                        }

                        video_consumer_cond.wait(lk, [&] {
                            return video_queue.size() > 0;
                        });

                        while (video_queue.size() > 0) {
                            images_vec.emplace_back(std::move(video_queue.front()));
                            video_queue.pop();
                        }

                    } // mutex
                    video_producer_cond.notify_one();

                    for (const auto& image : images_vec) {

                        { // wait until frame must be presented

                            const auto start_of_consuming =
                                std::chrono::high_resolution_clock::now();
                            const long double sleep_time =
                                (image.dts * video_time_ratio.num /
                                 static_cast<long double>(video_time_ratio.den)) -
                                std::chrono::duration<long double>(start_of_consuming -
                                                                   play_start_time)
                                    .count();

                            std::this_thread::sleep_for(
                                std::chrono::duration<long double>(sleep_time));
                        } // wait until frame must be presented

                        const auto start_of_drawing = std::chrono::high_resolution_clock::now();
#ifdef __linux__
                        syscall(1, STDOUT_FILENO, image.esqmap.data(), image.esqmap.size());
#elif __APPLE__
                        syscall(4, STDOUT_FILENO, image.esqmap.data(), image.esqmap.size());
#endif
                    }
                }
            }

            catch (const std::exception&) {
                video_consumer_thread_eptr = std::current_exception();
            }
        });
        video_consumer_thread.detach();
    }

    bool exit_main = false;
    while (!exit_main) {
        if (video_consumer_thread_eptr) std::rethrow_exception(video_consumer_thread_eptr);
        if (video_producer_thread_eptr) std::rethrow_exception(video_producer_thread_eptr);

        if (audio_play_thread_eptr) std::rethrow_exception(audio_play_thread_eptr);
        if (audio_cleanup_thread_eptr) std::rethrow_exception(audio_cleanup_thread_eptr);

        if (video_consumer_done) exit_main = true;

        std::this_thread::sleep_for(50ms);
    }
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
    Terminal::enable_stdout();
    Terminal::stop_tui_mode();
}
