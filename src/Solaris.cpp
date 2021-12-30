#include "Solaris.hpp"

#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "terminal/Terminal.hpp"

#include "openal/Device.hpp"
#include "openal/StreamingSource.hpp"

#include "ffmpeg/MediaLoader.hpp"
#include "ffmpeg/convert_audio_buffer.hpp"
#include "ffmpeg/scale_video_buffer.hpp"
#include "ffmpeg/shrink_audio_size_to_content.hpp"

#include "draw_frame_to_terminal.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;

Solaris::Solaris(const int argc, const char** const argv) {

    //@TODO argument parsing
    if (argc == 1) {
        throw std::runtime_error("no input file");
    }

    const std::filesystem::path filepath = argv[1];

    const bool path_valid = std::filesystem::is_regular_file(filepath);
    check(path_valid, "incorrect file path :" + filepath.string());

    filepath_ = std::move(filepath);
}
void Solaris::run() {

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::audio>> audio_loader;
    try {
        audio_loader.emplace(filepath_);
    } catch (const std::exception& e) {
        audio_loader.reset();

        print_msg_prefix();
        Terminal::out(RGB_t{200, 180, 0},
                      " audio disabled ",
                      Terminal::DefaultAttr,
                      "because : ",
                      e.what());
    }

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::video>> video_loader;
    try {
        video_loader.emplace(filepath_);
    } catch (const std::exception& e) {
        video_loader.reset();

        print_msg_prefix();
        Terminal::out(RGB_t{200, 180, 0},
                      " video disabled ",
                      Terminal::DefaultAttr,
                      "because : ",
                      e.what());
    }
    check(audio_loader.has_value() || video_loader.has_value(),
          "input file constains nor audio nor video");

    Terminal::start_tui_mode();

    std::signal(SIGWINCH, [](int) {
        Terminal::update_size();
    });
    std::signal(SIGINT, [](int) {
        throw std::runtime_error("sigint");
    });

    Terminal::flush();

    OpenAl::Device::get_singleton();
    OpenAl::StreamingSource sound_source;

    bool exit = false;
    while (!exit) {
        for (int i = 0; i < 200; i++) {

            if (audio_loader->decode_next_packet() == false) {
                exit = true;
            }
            if (video_loader->decode_next_packet() == false) {
                exit = true;
            }
        }
        auto audio_buffer = audio_loader->pop_buffer();
        auto video_buffer = video_loader->pop_buffer();

        FFmpeg::convert_audio_buffer_format(audio_buffer.begin(),
                                            audio_buffer.end(),
                                            AV_SAMPLE_FMT_FLT);

#if 0
        FFmpeg::scale_video_buffer(video_buffer.begin(),
                                   video_buffer.end(),
                                   Terminal::get_size().ws_col,
                                   Terminal::get_size().ws_row);
#endif

        draw_frame_to_terminal(std::move(*video_buffer.begin()), {1, 1});

        for (auto& elem : audio_buffer) {
            FFmpeg::shrink_audio_size_to_content(elem.ptr());
            sound_source.add_buffer(OpenAl::Buffer(std::move(elem)));
        }
        sound_source.play(audio_loader->get_time_ratio());
    }
}
void Solaris::print_msg_prefix() {

    Terminal::out("\n",
                  RGB_t{0, 200, 200},
                  "[[",
                  Terminal::DefaultAttr,
                  "Solaris",
                  RGB_t{0, 200, 200},
                  "]]",
                  Terminal::DefaultAttr,
                  "~~>");
}
Solaris::~Solaris() {
    Terminal::connect_stdout();
    Terminal::stop_tui_mode();
}
