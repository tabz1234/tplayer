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
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

#include "../RGB.hpp"

namespace Terminal {

    void set_fg_color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept;
    void reset_attributes() noexcept;

    void flush() noexcept;

    void redirect_stderr() noexcept;
    void connect_stderr() noexcept;

    void redirect_stdout() noexcept;
    void connect_stdout() noexcept;

    void clear() noexcept;

    void stop_tui_mode();
    void start_tui_mode();

    void update_size();

    std::pair<int, int> get_size();

    // fast low-level output
    inline void write_str(std::string_view str) noexcept;
    inline void write_char(const char ch) noexcept;
    // fast low-level output

    // slow high-level output

    using DefaultAttrT = enum class DefaultAttr_E_ : char {};
    static constexpr auto DefaultAttr = DefaultAttrT{};

    static void out() {
        flush();
    }
    static void out(DefaultAttr_E_) {
        reset_attributes();
    }
    static void out(RGB_t color) {
        set_fg_color(get<0>(color), get<1>(color), get<2>(color));
    }
    template <typename T>
    void out(T cur) {
        std::cout << cur;
        flush();
    }

    template <typename T, typename... Args>
    void out(T cur, Args... arguments) {
        out(cur);
        out(arguments...);
    }
    // slow high-level output

    struct Cursor final {

        static Cursor& get_singleton();

        void move(const int x, const int y) noexcept;

        void hide() noexcept;
        void show() noexcept;

        void shift_left() noexcept;
        void shift_right() noexcept;
        void shift_up() noexcept;
        void shift_down() noexcept;

        std::pair<int, int> get_pos() const noexcept;

      private:
        std::optional<std::pair<int, int>> pos_;
    };

} // namespace Terminal
