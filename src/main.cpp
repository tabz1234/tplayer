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

int
main(int argc, char** argv)
{

    if (argc == 1) {
        std::cout << "no file,aborting\n";
        std::exit(EXIT_FAILURE);
    }

    std::filesystem::path filepath = argv[1];
    if (!std::filesystem::is_regular_file(filepath)) {

        std::cout << "Incorrect file path :" << filepath;
        std::exit(EXIT_FAILURE);
    }

    std::signal(SIGINT, [](int) { throw std::runtime_error("sigint"); });

#if 0
    VideoLoaderDevice video_loader(std::filesystem::absolute(filepath));
    std::cout << video_loader.get_frames(100).capacity();

    auto& terminal = TerminalEmulator::get_singleton();
    terminal.start_tui_mode();
#endif

    openal::Device::get_singleton();
    openal::StreamingSource sound_source;

    FFmpeg::MediaLoader vloader(filepath);

    bool exit = false;
    while (!exit) {
        for (int i = 0; i < 500; i++) {

            if (vloader.decode_next_packet() == false) {
                exit = true;
            }
        }
#if 1
        FFmpeg::convert_audio_buffer_format(
          vloader.get_audio_buffer().begin(), vloader.get_audio_buffer().end(), AV_SAMPLE_FMT_FLT);
#endif

        for (auto& elem : vloader.get_audio_buffer()) {
            FFmpeg::shrink_audio_size_to_content(elem.ptr());
            sound_source.add_buffer(openal::Buffer(std::move(elem)));
        }
        sound_source.play(vloader.get_audio_ratio());

        vloader.clear_audio_buffer();
        vloader.clear_video_buffer();
    }
    /*@TODO list -> queue*/
    return EXIT_SUCCESS;
}
