#include "Settings.hpp"
#include "compute_scaling_factor.hpp"
#include "end_write_frame.hpp"
#include "init_cache_space.hpp"
#include "parse_cmd_arguments.hpp"
#include "write_audio_frame.hpp"
#include "write_video_frame.hpp"

#include "../ffmpeg/Codec.hpp"
#include "../ffmpeg/Format.hpp"
#include "../ffmpeg/Frame.hpp"
#include "../ffmpeg/HWAccelCodec.hpp"
#include "../ffmpeg/Packet.hpp"
#include "../ffmpeg/SWResampler.hpp"
#include "../ffmpeg/SWScaler.hpp"
#include "../ffmpeg/apply_ratio.hpp"
#include "../ffmpeg/decode_multi_packet.hpp"
#include "../ffmpeg/decode_packet.hpp"
#include "../ffmpeg/read_packet.hpp"
#include "../ffmpeg/resample_audio_frame.hpp"
#include "../ffmpeg/scale_video_frame.hpp"
#include "../ffmpeg/shrink_audio_size_to_content.hpp"

#include "../openal/Device.hpp"

#include "../util/DebugVariable.hpp"
#include "../util/fcheck.hpp"
#include "../util/integer_to_chars.hpp"

#include "../terminal/get_window_size.hpp"

#include "../fs/load_file_str.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <deque>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using namespace tplayer;

constexpr auto VIDEO_FRAMES_IN_FLIGHT = 3;

constexpr auto AUDIO_FRAMES_IN_FLIGHT = 20;

constexpr auto MAX_CACHED_VIDEO_FRAMES = 200;
constexpr auto VIDEO_CACHE_OVERSATURATED_TIMEOUT = std::chrono::milliseconds(1);

constexpr auto MAX_CACHED_AUDIO_FRAMES = 200;
constexpr auto AUDIO_CACHE_OVERSATURATED_TIMEOUT = std::chrono::milliseconds(1);

constexpr auto LOADER_STARVE_TIMEOUT = std::chrono::milliseconds(10);

constexpr std::string_view CACHE_VIDEO_DIR_NAME = "video";
constexpr std::string_view CACHED_VIDEO_FRAME_NAME = "rgb0_image";

constexpr std::string_view CACHE_AUDIO_DIR_NAME = "audio";
constexpr std::string_view CACHED_AUDIO_SAMPLE_NAME = "FLT_sample";

int main(const int argc, const char** const argv)
{
    //
    // APPLICATION WIDE INIT
    //

    Settings app_settings;
    parse_cmd_arguments(argc, argv, app_settings);

    OpenAL::Device openal_dev;

    //
    // FOR CACHE WRITES
    //
    std::string cur_cached_video_frame_path;
    cur_cached_video_frame_path.reserve(CACHE_VIDEO_DIR_NAME.size() + 32 + CACHED_VIDEO_FRAME_NAME.size());
    cur_cached_video_frame_path += CACHE_VIDEO_DIR_NAME;
    cur_cached_video_frame_path += '/';
    const auto cached_video_slash_iter = cur_cached_video_frame_path.end();

    std::string cur_cached_audio_frame_path;
    cur_cached_audio_frame_path.reserve(CACHE_AUDIO_DIR_NAME.size() + 32 + CACHED_AUDIO_SAMPLE_NAME.size());
    cur_cached_audio_frame_path += CACHE_AUDIO_DIR_NAME;
    cur_cached_audio_frame_path += '/';
    const auto cached_audio_slash_iter = cur_cached_audio_frame_path.end();

    //
    // FOR LOADER OPERATIONS
    //
    std::string cur_loading_video_frame_path_str;
    cur_loading_video_frame_path_str.reserve(CACHE_VIDEO_DIR_NAME.size() + 32 + CACHED_VIDEO_FRAME_NAME.size());
    cur_loading_video_frame_path_str += cur_cached_video_frame_path;
    const auto loading_video_slash_iter = cur_loading_video_frame_path_str.end();

    std::string cur_loading_audio_frame_path_str;
    cur_loading_audio_frame_path_str.reserve(CACHE_AUDIO_DIR_NAME.size() + 32 + CACHED_AUDIO_SAMPLE_NAME.size());
    cur_loading_audio_frame_path_str += cur_cached_audio_frame_path;
    const auto loading_audio_slash_iter = cur_loading_audio_frame_path_str.end();

    for (const auto file : app_settings.files_to_process) {

        //
        // PER-FILE INIT
        //

        std::mutex cached_video_props_mutex;

        std::deque<unsigned long> cached_video_ids;
        std::deque<double> cached_video_durations;
        std::deque<long> cached_video_sizes;

        std::mutex audio_props_mutex;

        std::deque<long> cached_audio_ids;
        std::deque<long> cached_audio_sizes;
        std::deque<long> cached_audio_durations;

        std::atomic<bool> producer_done = false;

        std::thread producer_thread([&] {
            //
            // PRODUCER_INIT
            //

            FFmpeg::Format format(file);
            if (!format.valid()) [[unlikely]] {
                fprintf(stderr, "open input failed\n");
                producer_done = true;
                return;
            }

            avformat_find_stream_info(format.get(), nullptr);
            av_dump_format(format.get(), 0, file.data(), 0);

            std::optional<int> video_stream_index;
            std::optional<FFmpeg::Codec> video_codec;
            std::optional<FFmpeg::HWAccelCodec> hw_video_codec;

            const auto construct_software_video_codec = [&] {
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

            std::optional<int> audio_stream_index;
            std::optional<FFmpeg::Codec> audio_codec;
            SwrContext* audio_resampler = nullptr;

            const auto a_stream = av_find_best_stream(format.get(), AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
            if (a_stream >= 0) {
                audio_stream_index = a_stream;
                const auto codecpar = format.get()->streams[audio_stream_index.value()]->codecpar;
                audio_codec.emplace(codecpar);
                audio_resampler = FFmpeg::create_swr_resampler(codecpar->channel_layout,
                                                               static_cast<enum AVSampleFormat>(codecpar->format),
                                                               codecpar->sample_rate,
                                                               codecpar->channel_layout,
                                                               AV_SAMPLE_FMT_FLT,
                                                               codecpar->sample_rate);
            }

            const bool use_hw_video_routines = hw_video_codec.has_value() && hw_video_codec->valid();
            const bool use_video_routines = video_codec.has_value() && video_codec->valid();
            const bool use_audio_routines = audio_codec.has_value() && audio_codec->valid() && openal_dev.valid();

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

            unsigned long cur_video_frame_id = 0;

            constexpr auto AUDIO_FRAMES_VEC_RESERVE = 30;
            std::vector<FFmpeg::Frame> raw_audio_frames_vec(AUDIO_FRAMES_VEC_RESERVE);

            FFmpeg::Frame final_audio_frame;

            unsigned long cur_audio_frame_id = 0;

            const auto cache_space_created = init_cache_space();
            fcheck(cache_space_created == 0, [] {
                fprintf(stderr, "init_cache_space failed, TODO fallback to in-memory buffering");
                exit(EXIT_FAILURE);
            });

            //
            // PRODUCER_LOOP
            //

            while (!producer_done) {

                const auto read_ret = FFmpeg::read_packet(format, packet);
                if (read_ret != 0) [[unlikely]] {
                    producer_done = true;
                    break;
                }

                if (use_hw_video_routines && packet.handle->stream_index == video_stream_index.value()) {

                    const auto decode_ret = FFmpeg::decode_packet(hw_video_codec.value(), packet, raw_hw_temp_farme, raw_video_frame);

                    raw_video_frame.wipe();
                }
                else if (use_video_routines && packet.handle->stream_index == video_stream_index.value()) {

                    while (cached_video_ids.size() >= MAX_CACHED_VIDEO_FRAMES) {
                        std::this_thread::sleep_for(VIDEO_CACHE_OVERSATURATED_TIMEOUT);
                    }

                    const auto decode_ret = FFmpeg::decode_packet(video_codec.value(), packet, raw_video_frame);
                    if (decode_ret != 0) [[unlikely]] {
                        fprintf(stderr,
                                "Abnormal sw video decode ret :%d, cur id :%ld, skiped this packet\n",
                                decode_ret,
                                cur_video_frame_id);
                        continue;
                    }

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

                    std::error_code errc;
                    std::array<char, 64> charconv_buff;
                    cur_cached_video_frame_path.erase(cached_video_slash_iter, cur_cached_video_frame_path.end());
                    cur_cached_video_frame_path += integer_to_chars(cur_video_frame_id, charconv_buff.data(), charconv_buff.size());
                    std::filesystem::create_directory(cur_cached_video_frame_path, errc);
                    cur_cached_video_frame_path += '/';
                    cur_cached_video_frame_path += CACHED_VIDEO_FRAME_NAME;

                    const auto write_ret = write_video_frame(final_video_frame, cur_cached_video_frame_path);
                    if (write_ret != 0) [[unlikely]] {
                        fprintf(stderr, "write_video_frame fail, errno :%d\n", errno);
                    }
                    else {
                        cached_video_props_mutex.lock();

                        cached_video_ids.emplace_back(cur_video_frame_id);
                        cached_video_durations.emplace_back(
                            FFmpeg::apply_ratio(final_video_frame.handle->pkt_duration,
                                                format.get()->streams[video_stream_index.value()]->time_base));
                        cached_video_sizes.emplace_back(final_video_frame.handle->linesize[0] * final_video_frame.handle->height);

                        cached_video_props_mutex.unlock();

                        cur_video_frame_id++;
                    }

                    final_video_frame.wipe();
                }
                else if (use_audio_routines && packet.handle->stream_index == audio_stream_index.value()) {

                    while (cached_audio_ids.size() >= MAX_CACHED_AUDIO_FRAMES) {
                        std::this_thread::sleep_for(AUDIO_CACHE_OVERSATURATED_TIMEOUT);
                    }

                    const auto decode_ret = FFmpeg::decode_multi_packet(audio_codec.value(), packet, raw_audio_frames_vec);
                    if (decode_ret != -11) [[unlikely]] {
                        fprintf(stderr, "Abnormal audio decode ret :%d, cur id :%ld, skiped this packet\n", decode_ret, cur_audio_frame_id);
                        continue;
                    }

                    for (int i = 0; i < raw_audio_frames_vec.size(); ++i) {

                        if (raw_audio_frames_vec.at(i).handle->data[0] == nullptr) {
                            break;
                        }

                        final_audio_frame.handle->format = AV_SAMPLE_FMT_FLT;
                        FFmpeg::resample_audio_frame(audio_resampler, raw_audio_frames_vec.at(i), final_audio_frame);

                        FFmpeg::shrink_audio_size_to_content(final_audio_frame);

                        std::error_code errc;
                        std::array<char, 64> charconv_buff;
                        cur_cached_audio_frame_path.erase(cached_audio_slash_iter, cur_cached_audio_frame_path.end());
                        cur_cached_audio_frame_path += integer_to_chars(cur_audio_frame_id, charconv_buff.data(), charconv_buff.size());
                        std::filesystem::create_directory(cur_cached_audio_frame_path, errc);
                        cur_cached_audio_frame_path += '/';
                        cur_cached_audio_frame_path += CACHED_AUDIO_SAMPLE_NAME;

                        const auto write_ret = write_audio_frame(final_audio_frame, cur_cached_audio_frame_path);
                        if (write_ret != 0) [[unlikely]] {
                            fprintf(stderr, "write_audio_frame fail, errno :%d\n", errno);
                        }
                        else {
                            audio_props_mutex.lock();

                            cached_audio_ids.emplace_back(cur_audio_frame_id);
                            cached_audio_sizes.emplace_back(final_audio_frame.handle->linesize[0]);

                            audio_props_mutex.unlock();

                            cur_audio_frame_id++;
                        }

                        final_audio_frame.wipe();
                    }
                }

                packet.wipe();
            }

            //
            // PRODUCER CLEANUP
            //

            FFmpeg::destroy_swr_resampler(audio_resampler);
            FFmpeg::destroy_sws_scaler(video_scaler);
        });

        //
        // LOADER INIT
        //

        //
        // VIDEO PRESENTATION QUEUE(DEQUEUE)
        //
        std::deque<std::string> presentation_images;
        presentation_images.resize(VIDEO_FRAMES_IN_FLIGHT);

        for (auto& image : presentation_images) {
            image.reserve(1000000);
        }

        //
        // VIDEO LOADING VARS
        //
        std::mutex video_loading_data_mutex;

        std::deque<unsigned long> video_loading_ids;

        std::deque<double> video_loading_durations;

        std::atomic<bool> video_loader_done = false;
        std::thread video_loader_thread([&] {
            //
            // VIDEO LOADER LOOP
            //

            while (!video_loader_done) {

                //
                // IF VIDEO LOADER STARVING
                //

                if (cached_video_ids.empty()) [[unlikely]] {
                    if (producer_done) {
                        video_loader_done = true;
                        break;
                    }
                    else {
                        std::this_thread::sleep_for(LOADER_STARVE_TIMEOUT);
                        fprintf(stderr, "sleeped STARVE\n");
                        continue;
                    }
                }

                if (video_loading_ids.size() >= VIDEO_FRAMES_IN_FLIGHT) {
                    std::this_thread::sleep_for(std::chrono::duration<double>(video_loading_durations.front()));
                    fprintf(stderr, "sleeped\n");
                }

                //
                // READ METADATA OF CAHED FRAME
                //
                cached_video_props_mutex.lock();
                const auto loading_frame_id = cached_video_ids.front();
                const auto loading_frame_duration = cached_video_durations.front();
                const auto loading_frame_size = cached_video_sizes.front();
                cached_video_props_mutex.unlock();

                std::array<char, 64> charconv_buff;
                cur_loading_video_frame_path_str.erase(loading_video_slash_iter, cur_loading_video_frame_path_str.end());
                cur_loading_video_frame_path_str += integer_to_chars(loading_frame_id, charconv_buff.data(), charconv_buff.size());
                cur_loading_video_frame_path_str += '/';
                cur_loading_video_frame_path_str += CACHED_VIDEO_FRAME_NAME;

                auto& dst_image = presentation_images.back();
                const auto actual_size = fs::load_file_str(cur_loading_video_frame_path_str, loading_frame_size, dst_image.data());
                if (actual_size != loading_frame_size) [[unlikely]] {
                    fprintf(stderr,
                            "Incorrect frame read, desired size :%ld, actual size :%ld, skip this frame\n",
                            loading_frame_size,
                            actual_size);

                    //
                    // POP CACHED METADATA
                    //
                    cached_video_props_mutex.lock();
                    cached_video_ids.pop_front();
                    cached_video_durations.pop_front();
                    cached_video_sizes.pop_front();
                    cached_video_props_mutex.unlock();
                }
                else {
                    //
                    // PUSH LOADED METADATA
                    //
                    video_loading_data_mutex.lock();
                    video_loading_durations.emplace_back(loading_frame_duration);
                    video_loading_ids.emplace_back(loading_frame_id);
                    video_loading_data_mutex.unlock();

                    //
                    // POP CACHED METADATA
                    //
                    cached_video_props_mutex.lock();
                    cached_video_ids.pop_front();
                    cached_video_durations.pop_front();
                    cached_video_sizes.pop_front();
                    cached_video_props_mutex.unlock();

                    std::error_code errc;
                    std::filesystem::remove(cur_loading_video_frame_path_str, errc);
                }
            }
        });

        while (1) {
            printf("MAIN LOOOP\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        producer_thread.join();
        video_loader_thread.join();
    }

    return 0;
}
