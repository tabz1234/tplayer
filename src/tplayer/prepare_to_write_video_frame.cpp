#include "prepare_to_write_video_frame.hpp"

#include <array>
#include <filesystem>

#include "../util/integer_to_chars.hpp"

namespace tplayer {
    std::error_code prepare_to_write_video_frame(const unsigned long id) noexcept
    {
        std::error_code ret;

        std::array<char, 255> charconv_buff;
        const auto id_sv = integer_to_chars(id, charconv_buff.data(), charconv_buff.size());

        std::filesystem::current_path("video/", ret);
        if (ret) [[unlikely]]
            return ret;
        std::filesystem::create_directory(id_sv, ret);
        if (ret) [[unlikely]]
            return ret;
        std::filesystem::current_path(id_sv, ret);

        return ret;
    }
} // namespace tplayer
