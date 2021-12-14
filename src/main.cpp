#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "terminal/TerminalEmulator.hpp"

#include "openal/OpenAlDevice.hpp"
#include "openal/OpenAlStreamingSource.hpp"

#include "ffmpeg/FFmpegAudioFormatConverter.hpp"
#include "ffmpeg/FFmpegVideoLoader.hpp"

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

    std::signal(SIGINT, [](int) { std::exit(EXIT_FAILURE); });

#if 0
    VideoLoaderDevice video_loader(std::filesystem::absolute(filepath));
    std::cout << video_loader.get_frames(100).capacity();

    auto& terminal = TerminalEmulator::get_singleton();
    terminal.start_tui_mode();
#endif

    openal::Device::get_singleton();
    openal::StreamingSource sound_source;

    ffmpeg::MediaLoader vloader(filepath);

    bool exit = false;
    while (!exit) {
        for (int i = 0; i < 500; i++) {

            if (vloader.decode_next_packet() == false) {
                exit = true;
            }
        }
        convert_audio_buffer_format(
          vloader.get_audio_buffer().begin(), vloader.get_audio_buffer().end(), AV_SAMPLE_FMT_FLT);

        for (auto& elem : vloader.get_audio_buffer()) {
            sound_source.add_buffer(openal::Buffer(std::move(elem)));
        }
        sound_source.play(vloader.get_audio_ratio());

        vloader.clear_audio_buffer();
        vloader.clear_video_buffer();
    }
    /*@TODO list -> queue*/
    return EXIT_SUCCESS;
}
