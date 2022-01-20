#pragma once

#include <string_view>

namespace FFmpeg {

    enum class MediaType : char {
        audio = 1,
        video = 0,
        subtitle = 3

    };

    constexpr static std::string_view MediaType_to_sv(const MediaType media_type) {
        switch (media_type) {
        case MediaType::audio: return "audio";
        case MediaType::video: return "video";
        case MediaType::subtitle: return "subtitle";
        };
    }
} // namespace FFmpeg
