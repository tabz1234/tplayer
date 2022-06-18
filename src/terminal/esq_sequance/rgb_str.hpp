#pragma once

#include <array>
#include <optional>
#include <string>

#include "../../util/integer_to_chars.hpp"
#include "header.hpp"

enum class Scene { FG, BG };

namespace terminal::esq_sequance {
    template <Scene V>
    void rgb_str(const uint8_t r, const uint8_t g, const uint8_t b, std::string& out) noexcept
    {
        std::array<char, 4> charconv_buff;

        out += header;
        out += integer_to_chars(r, charconv_buff.data(), charconv_buff.size());
        out += ";";
        out += integer_to_chars(g, charconv_buff.data(), charconv_buff.size());
        out += ";";
        out += integer_to_chars(b, charconv_buff.data(), charconv_buff.size());
        out += "m";
    }
} // namespace terminal::esq_sequance
