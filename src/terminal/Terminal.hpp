#pragma once

extern "C" {
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
}

#include <charconv>
#include <concepts>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "../RGB.hpp"

namespace Terminal {

    void set_fg_color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept;
    void reset_attributes() noexcept;

    void flush() noexcept;

    void suspend_stderr() noexcept;
    void enable_stderr() noexcept;

    void suspend_stdout() noexcept;
    void enable_stdout() noexcept;

    void clear() noexcept;

    void stop_tui_mode();
    void start_tui_mode();

    void update_size();

    struct winsize get_size();

    // fast low-level output
    void write_str(std::string_view str) noexcept;
    void write_char(const char ch) noexcept;
    // fast low-level output

    // slow high-level output

    using DefaultAttrT = enum class DefaultAttr_E_ : char {};
    static constexpr auto DefaultAttr = DefaultAttrT{};
    static constexpr auto newl = '\n';

    static void out(DefaultAttr_E_)
    {
        reset_attributes();
    }
    static void out(RGB_t color)
    {
        set_fg_color(get<0>(color), get<1>(color), get<2>(color));
    }
    template <typename T>
    void out(T cur)
    {
        std::cout << cur;
        flush();
    }
    template <typename T, typename... Args>
    void out(T cur, Args... arguments)
    {

        out(cur);
        out(arguments...);

        reset_attributes();
    }

    // slow high-level output

    namespace Cursor {

        void move(const int x, const int y) noexcept;

        void hide() noexcept;
        void show() noexcept;

        void shift_left() noexcept;
        void shift_right() noexcept;
        void shift_up() noexcept;
        void shift_down() noexcept;
    }; // namespace Cursor

} // namespace Terminal
