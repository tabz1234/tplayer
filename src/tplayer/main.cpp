#include "Settings.hpp"
#include "parse_cmd_arguments.hpp"

#include "../ffmpeg/Codec.hpp"
#include "../ffmpeg/Format.hpp"
#include "../ffmpeg/Frame.hpp"
#include "../ffmpeg/HWAccelCodec.hpp"
#include "../ffmpeg/Packet.hpp"
#include "../ffmpeg/decode_video_packet.hpp"
#include "../ffmpeg/read_packet.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <thread>

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

            avformat_find_stream_info(format.get(), nullptr);
            av_dump_format(format.get(), 0, file.data(), 0);

            std::optional<int> video_stream_index;
            std::optional<FFmpeg::Codec> video_codec;
            std::optional<FFmpeg::HWAccelCodec> hw_video_codec;

            std::optional<int> audio_stream_index;
            std::optional<FFmpeg::Codec> audio_codec;

            const auto v_stream = av_find_best_stream(format.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
            if (v_stream >= 0) {
                video_stream_index = v_stream;

                hw_video_codec.emplace(format.get()->streams[video_stream_index.value()]->codecpar,
                                       static_cast<enum AVHWDeviceType>(app_settings.hw_dev));

                if (!hw_video_codec.value().valid()) {
                    fprintf(stderr, "VAAPI decoder fail, fallback to software decoding\n");

                    video_codec.emplace(format.get()->streams[video_stream_index.value()]->codecpar);
                    if (!video_codec.value().valid()) [[unlikely]] {
                        fprintf(stderr, "Software video decoder fail, exiting\n");
                        return;
                    }
                }
            }

            const auto a_stream = av_find_best_stream(format.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
            if (a_stream >= 0) {
                audio_stream_index = a_stream;
                audio_codec.emplace(format.get()->streams[audio_stream_index.value()]->codecpar);
            }

            const bool use_hw_video_routines =
                video_stream_index.has_value() && hw_video_codec.has_value() && hw_video_codec.value().valid();
            const bool use_video_routines = video_stream_index.has_value() && video_codec.has_value() && video_codec.value().valid();
            const bool use_audio_routines = audio_stream_index.has_value() && audio_codec->valid();

            FFmpeg::Packet packet;

            FFmpeg::Frame video_frame;
            FFmpeg::Frame hw_temp_farme;

            constexpr auto AUDIO_FRAMES_VEC_RESERVE = 100;
            std::vector<FFmpeg::Frame> audio_frames_vec;
            audio_frames_vec.reserve(AUDIO_FRAMES_VEC_RESERVE);

            while (!producer_need_to_exit) {
                const auto read_ret = FFmpeg::read_packet(format, packet);
                if (read_ret != 0) {
                    producer_need_to_exit = true;
                    return;
                }

                if (use_hw_video_routines && packet.handle->stream_index == video_stream_index.value()) {
                    const auto decode_ret = FFmpeg::decode_video_packet(hw_video_codec.value(), packet, hw_temp_farme, video_frame);

                    video_frame.wipe();
                    hw_temp_farme.wipe();
                }

                if (use_video_routines && packet.handle->stream_index == video_stream_index.value()) {
                    const auto decode_ret = FFmpeg::decode_video_packet(video_codec.value(), packet, video_frame);

                    video_frame.wipe();
                }

                if (use_audio_routines && packet.handle->stream_index == audio_stream_index.value()) {
                    audio_frames_vec.clear();
                }

                packet.wipe();
            }
        });

        producer_thread.join();
    }

    return 0;
}
