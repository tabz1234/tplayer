#pragma once

extern "C"
{
#include <sys/ioctl.h>
#include <termios.h>
}

#include <optional>
#include <utility>

#include "RGB.hpp"

namespace Terminal {

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
set_fg_color(const RGB& color) noexcept;
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

struct Cursor
{

    static Cursor& get_singleton();

    void move(const std::pair<int, int> new_pos) noexcept;

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
