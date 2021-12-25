#include "Solaris.hpp"

#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "terminal/TerminalEmulator.hpp"

#include "openal/Device.hpp"
#include "openal/StreamingSource.hpp"

#include "ffmpeg/MediaLoader.hpp"
#include "ffmpeg/convert_audio_buffer.hpp"
#include "ffmpeg/shrink_audio_size_to_content.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;

Solaris::Solaris(const int argc, const char** const argv)
{

    //@TODO argument parsing
    if (argc == 1) {
        throw std::runtime_error("no input file");
    }

    const std::filesystem::path filepath = argv[1];
    if (!std::filesystem::is_regular_file(filepath)) {
        throw std::runtime_error("incorrect file path :"s + filepath.c_str());
    }
    filepath_ = std::move(filepath);

    Terminal::start_tui_mode();

    std::signal(SIGINT, [](int) { throw std::runtime_error("sigint"); });
}

void
Solaris::run()
{

    std::signal(SIGWINCH, [](int) { Terminal::update_size(); });

    Terminal::turn_off_stdout();
    Terminal::turn_off_stderr();

    Terminal::flush();

    std::optional<FFmpeg::MediaLoader<FFmpeg::MediaType::audio>> audio_loader;
    try {
        audio_loader.emplace(filepath_);
    } catch (const std::exception& e) {
        Terminal::set_fg_color({ 0, 222, 255 });

        std::cout << "\r\nWARNING : audio disabled because : " << e.what() << "\r\n";

        Terminal::reset_attributes();
    }

    OpenAl::Device::get_singleton();
    OpenAl::StreamingSource sound_source;

    bool exit = false;
    while (!exit) {
        for (int i = 0; i < 2000; i++) {

            if (audio_loader->decode_next_packet() == false) {
                exit = true;
            }
        }
        auto audio_buffer = audio_loader->pop_buffer();

        FFmpeg::convert_audio_buffer_format(audio_buffer.begin(), audio_buffer.end(), AV_SAMPLE_FMT_FLT);

        for (auto& elem : audio_buffer) {
            FFmpeg::shrink_audio_size_to_content(elem.ptr());
            sound_source.add_buffer(OpenAl::Buffer(std::move(elem)));
        }
        sound_source.play(audio_loader->get_time_ratio());
    }
}

Solaris::~Solaris()
{
    Terminal::stop_tui_mode();
}
