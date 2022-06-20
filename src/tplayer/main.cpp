#include "Settings.hpp"
#include "compute_scaling_factor.hpp"
#include "init_cache_space.hpp"
#include "parse_cmd_arguments.hpp"

#include "../ffmpeg/Codec.hpp"
#include "../ffmpeg/Format.hpp"
#include "../ffmpeg/Frame.hpp"
#include "../ffmpeg/HWAccelCodec.hpp"
#include "../ffmpeg/Packet.hpp"
#include "../ffmpeg/SWScaler.hpp"
#include "../ffmpeg/decode_multi_packet.hpp"
#include "../ffmpeg/decode_packet.hpp"
#include "../ffmpeg/read_packet.hpp"
#include "../ffmpeg/scale_video_frame.hpp"

#include "../util/fcheck.hpp"

#include "../terminal/get_window_size.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdio>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

using namespace tplayer;

int main(const int argc, const char** const argv)
{
    Settings app_settings;
    parse_cmd_arguments(argc, argv, app_settings);

    std::atomic<bool> producer_need_to_exit = false;

    for (const auto file : app_settings.files_to_process) {

        std::thread producer_thread([&] {
            FFmpeg::Format format(file);
            if (!format.valid()) [[unlikely]] {
                fprintf(stderr, "open input failed\n");

                producer_need_to_exit = true;
                return;
            }

            // freopen("stderr.dump", "w+", stderr);
            // freopen("stdout.dump", "w+", stdout);

            avformat_find_stream_info(format.get(), nullptr);
            av_dump_format(format.get(), 0, file.data(), 0);

            std::optional<int> video_stream_index;
            std::optional<FFmpeg::Codec> video_codec;
            std::optional<FFmpeg::HWAccelCodec> hw_video_codec;

            std::optional<int> audio_stream_index;
            std::optional<FFmpeg::Codec> audio_codec;

            auto construct_software_video_codec = [&] {
                video_codec.emplace(format.get()->streams[video_stream_index.value()]->codecpar);
                if (!video_codec.value().valid()) [[unlikely]] {
                    fprintf(stderr, "Software video decoder init failed \n");
                    return;
                }
            };

            const auto v_stream = av_find_best_stream(format.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
            if (v_stream >= 0) {
                video_stream_index = v_stream;

                if (app_settings.hw_dev == HWAccelDevice::VAAPI) {
                    hw_video_codec.emplace(format.get()->streams[video_stream_index.value()]->codecpar,
                                           static_cast<enum AVHWDeviceType>(app_settings.hw_dev));

                    if (!hw_video_codec.has_value()) {
                        fprintf(stderr, "VAAPI video decoder init failed, fallback to software decoding\n");
                        construct_software_video_codec();
                    }
                }
                else {
                    construct_software_video_codec();
                }
            }

            const auto a_stream = av_find_best_stream(format.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
            if (a_stream >= 0) {
                audio_stream_index = a_stream;
                audio_codec.emplace(format.get()->streams[audio_stream_index.value()]->codecpar);
            }

            const bool use_hw_video_routines = hw_video_codec.has_value() && hw_video_codec->valid();
            const bool use_video_routines = video_codec.has_value() && video_codec->valid();
            const bool use_audio_routines = audio_codec.has_value() && audio_codec->valid();

            FFmpeg::Packet packet;

            FFmpeg::Frame raw_video_frame;
            FFmpeg::Frame raw_hw_temp_farme;

            int prev_width = 0;
            int prev_height = 0;

            int new_width = 0;
            int new_height = 0;
            const auto new_format = AV_PIX_FMT_RGB0;

            int prev_window_width = 0;
            int prev_window_height = 0;

            SwsContext* video_scaler = nullptr;
            FFmpeg::Frame final_video_frame;

            constexpr auto AUDIO_FRAMES_VEC_RESERVE = 100;
            std::vector<FFmpeg::Frame> audio_frames_vec(AUDIO_FRAMES_VEC_RESERVE);

            init_cache_space();

            while (!producer_need_to_exit) {
                const auto read_ret = FFmpeg::read_packet(format, packet);
                if (read_ret != 0) {
                    producer_need_to_exit = true;
                    return;
                }

                if (use_hw_video_routines && packet.handle->stream_index == video_stream_index.value()) {
                    const auto decode_ret = FFmpeg::decode_packet(hw_video_codec.value(), packet, raw_hw_temp_farme, raw_video_frame);

                    raw_video_frame.wipe();
                }
                else if (use_video_routines && packet.handle->stream_index == video_stream_index.value()) {
                    const auto decode_ret = FFmpeg::decode_packet(video_codec.value(), packet, raw_video_frame);

                    const auto current_window_extent = terminal::get_window_size();
                    const auto scaler = compute_scaling_factor(current_window_extent.ws_xpixel,
                                                               current_window_extent.ws_ypixel,
                                                               raw_video_frame.handle->width,
                                                               raw_video_frame.handle->height);

                    new_width = raw_video_frame.handle->width * scaler;
                    new_height = raw_video_frame.handle->height * scaler;

                    if (current_window_extent.ws_xpixel != prev_window_width || current_window_extent.ws_ypixel != prev_window_height ||
                        new_width != prev_width || new_height != prev_height) {

                        FFmpeg::destroy_sws_scaler(video_scaler);
                        video_scaler = FFmpeg::create_sws_scaler(raw_video_frame.handle->width,
                                                                 raw_video_frame.handle->height,
                                                                 static_cast<enum AVPixelFormat>(raw_video_frame.handle->format),
                                                                 new_width,
                                                                 new_height,
                                                                 new_format,
                                                                 0);
                    }

                    final_video_frame.handle->width = new_width;
                    final_video_frame.handle->height = new_height;
                    final_video_frame.handle->format = new_format;
                    FFmpeg::scale_video_frame(video_scaler, raw_video_frame, final_video_frame);

                    prev_width = new_width;
                    prev_height = new_height;

                    prev_window_width = current_window_extent.ws_xpixel;
                    prev_window_height = current_window_extent.ws_ypixel;

                    final_video_frame.wipe();
                }
                else if (use_audio_routines && packet.handle->stream_index == audio_stream_index.value()) {
                    const auto decode_ret = FFmpeg::decode_multi_packet(audio_codec.value(), packet, audio_frames_vec);

                    for (auto& audio_frame : audio_frames_vec) {
                    }
                }

                packet.wipe();
            }

            FFmpeg::destroy_sws_scaler(video_scaler);
        });

#if 0
        while (1) {
            printf("LOOOP\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

#endif
        producer_thread.join();
    }

    return 0;
}
