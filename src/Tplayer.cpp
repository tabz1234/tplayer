#include "Tplayer.hpp"

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <queue>
#include <stdexcept>
#include <thread>

#include "terminal/Terminal.hpp"

#include "openal/Device.hpp"
#include "openal/StreamingSource.hpp"

#include "ffmpeg/MediaLoader.hpp"
#include "ffmpeg/convert_audio_buffer.hpp"
#include "ffmpeg/scale_video_buffer.hpp"
#include "ffmpeg/shrink_audio_size_to_content.hpp"

#include "frame_to_esqmap.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;

Tplayer::Tplayer(const int argc, const char** const argv)
{

    //@TODO argument parsing
    if (argc == 1) {
        throw std::runtime_error("no input file");
    }

    filepath_ = argv[1];

    const bool path_valid = std::filesystem::is_regular_file(filepath_);
    check(path_valid, "incorrect file path :" + filepath_.string());

    std::signal(SIGWINCH, [](int) {
        Terminal::update_size();
    });
    std::signal(SIGINT, [](int) {
        Terminal::stop_tui_mode();

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

    constexpr auto CONSUME_COUNT = 2;
    constexpr auto PRODUCE_COUNT = CONSUME_COUNT * 10;

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::video>> video_loader;
    std::optional<std::queue<FFmpeg::Frame<FFmpeg::MediaType::video>>> video_queue;

    std::condition_variable producer_cond;
    std::condition_variable consumer_cond;
    std::mutex prod_cons_mutex;

    std::exception_ptr eptr;

    try {
        video_loader.emplace(filepath_.c_str());
        video_queue.emplace();
        Terminal::start_tui_mode();
    }
    catch (const std::exception& e) {
        print_disable_warning("video", e.what());
    }
#if 0
    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::audio>> audio_loader;
    std::optional<OpenAl::StreamingSource> sound_source;
    try {
        audio_loader.emplace(filepath_.c_str());
        OpenAl::Device::get_singleton();
        prod_cons_mutex.emplace();
        sound_source.emplace();
    }
    catch (const std::exception& e) {
        print_disable_warning("audio", e.what());
    }
#endif

    const bool produce_video = video_loader.has_value();
    std::atomic<bool> producer_alive = true;

    std::thread producer_thread([&] {
        try {
            while (producer_alive.load()) {
                std::vector<FFmpeg::Frame<FFmpeg::MediaType::video>> raw_frames_vec;
                raw_frames_vec.reserve(PRODUCE_COUNT);

                for (int i = 0; i < PRODUCE_COUNT; ++i) {

                    auto decoder_output = video_loader->decode_next_packet();
                    if (decoder_output.has_value() != true) {
                        producer_alive = false;
                        break;
                    }

                    for (auto&& raw_frame : decoder_output.value()) {
                        raw_frames_vec.emplace_back(std::move(raw_frame));
                    }
                }
                if (raw_frames_vec.size() < 1) continue;

                Terminal::update_size();
                auto rescaled_frames =
                    scale_video_buffer<decltype(raw_frames_vec)>(raw_frames_vec.cbegin(),
                                                                 raw_frames_vec.cend(),
                                                                 Terminal::get_size().ws_col,
                                                                 Terminal::get_size().ws_row);
                {
                    std::unique_lock<std::mutex> lk{prod_cons_mutex};

                    producer_cond.wait(lk, [&] {
                        return video_queue->size() < PRODUCE_COUNT;
                    });
                    for (auto&& rescaled : rescaled_frames) {
                        video_queue.value().emplace(std::move(rescaled));
                    }
                } // mutex
                consumer_cond.notify_one();
            }
        }
        catch (...) {
            eptr = std::current_exception();
        }

        consumer_cond.notify_all();
    });

    const auto play_start_time = std::chrono::high_resolution_clock::now();
    const auto video_time_ratio = video_loader->get_time_ratio();

    bool exit = false;
    while (!exit) {
        std::vector<FFmpeg::Frame<FFmpeg::MediaType::video>> frames_vec;
        frames_vec.reserve(CONSUME_COUNT);

        {
            std::unique_lock<std::mutex> lk{prod_cons_mutex};

            consumer_cond.wait(lk, [&] {
                return video_queue->size() >= CONSUME_COUNT || !producer_alive.load();
            });

            if (!producer_alive.load()) exit = true;
            if (eptr) std::rethrow_exception(eptr);

            while (video_queue->size() > 0) {
                frames_vec.emplace_back(std::move(video_queue->front()));
                video_queue->pop();
            }

        } // mutex
        producer_cond.notify_one();

        for (const auto& cur_frame : frames_vec) {

            const auto play_offset_time =
                std::chrono::high_resolution_clock::now() - play_start_time;
            const long double sleep_time =
                (cur_frame.ptr()->pts * video_time_ratio.num /
                 static_cast<long double>(video_time_ratio.den)) -
                std::chrono::duration<long double>(play_offset_time).count();

            if (sleep_time > 0) {
                std::this_thread::sleep_for(std::chrono::duration<long double>(sleep_time));
            }

            const auto cur_esqmap = frame_to_esqmap(cur_frame.ptr());

            Terminal::Cursor::move(1, 1);
            write(STDOUT_FILENO, cur_esqmap.data(), cur_esqmap.size());
        }
    }

    producer_thread.join();

#if 0
    bool exit = false;
    while (!exit) {
        for (int i = 0; i < 20; i++) {

            if (audio_loader->decode_next_packet() == false) {
                exit = true;
            }
            if (video_loader->decode_next_packet() == false) {
                exit = true;
            }
        } 

        const auto rescaled_video_buffer = FFmpeg::scale_video_buffer(video_loader->cbegin(),
                                                                      video_loader->cend(),
                                                                      Terminal::get_size().ws_col,
                                                                      Terminal::get_size().ws_row);
        video_loader->clear();

        std::vector<std::string> esqmap_buffer;
        esqmap_buffer.reserve(20);

        for (const auto& rescaled_frame : rescaled_video_buffer) {
            esqmap_buffer.emplace_back(frame_to_esqmap(rescaled_frame.ptr()));
        }

        for (const auto& esqmap : esqmap_buffer) {
            Terminal::Cursor::move(1, 1);
            write(STDOUT_FILENO, esqmap.data(), esqmap.size());
        }
        auto interleaved_audio_buffer = FFmpeg::convert_audio_buffer_format(audio_loader->cbegin(),
                                                                            audio_loader->cend(),
                                                                            AV_SAMPLE_FMT_FLT);

        for (auto& interleaved_frame : interleaved_audio_buffer) {
            FFmpeg::shrink_audio_size_to_content(interleaved_frame.ptr());
            sound_source.add_buffer(OpenAl::Buffer(std::move(interleaved_frame)));
        }
        sound_source.play(audio_loader->get_time_ratio());

        audio_loader->clear();
    }
#endif
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
