#pragma once

extern "C" {
#include <libavutil/samplefmt.h>
}

#include <string_view>
#include <unordered_map>

namespace FFmpeg {

    enum class MediaType {
        audio = AVMEDIA_TYPE_AUDIO,
        video = AVMEDIA_TYPE_VIDEO

    };

    const std::unordered_map<MediaType, std::string_view> MediaTypeStrings = {
        {MediaType::audio, "audio"},
        {MediaType::video, "video"}};

} // namespace FFmpeg
