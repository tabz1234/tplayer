#pragma once

extern "C"
{
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
}

#include <charconv>
#include <concepts>
#include <cstdio>
#include <iostream>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

namespace Terminal {

void
set_fg_color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept;

void
turn_off_stderr() noexcept;

void
turn_on_stderr() noexcept;

void
turn_off_stdout() noexcept;

void
turn_on_stdout() noexcept;

void
clear() noexcept;

void
flush() noexcept;

void
reset_attributes() noexcept;

void
stop_tui_mode();
void
start_tui_mode();

void
update_size();

std::pair<int, int>
get_size();

struct RGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

template<RGB color>
void
out()
{
    reset_attributes();
    flush();
}
template<RGB color, typename T, typename... Args>
auto
out(T first, Args... data)
{
    set_fg_color(color.r, color.g, color.b);
    std::cout << first; // cur in recursion
    flush();

    out<color>(data...);
}
static void
out()
{
    reset_attributes();
    flush();
}
template<typename T, typename... Args>
auto
out(T first, Args... data)
{
    std::cout << first; // cur in recursion

    out(data...);
}
struct Cursor
{

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

static auto& cursor = Cursor::get_singleton();

}
