#include "TerminalEmulator.hpp"

extern "C"
{
#include <unistd.h>
}

#include <csignal>
#include <iostream>

void
TerminalEmulator::stop_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios_) == -1) {
        throw std::runtime_error("tcsetattr orig failed");
    }
}

void
TerminalEmulator::start_raw_mode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios_) == -1)
        throw std::runtime_error("tcgetattr failed");

    cur_termios_ = orig_termios_;

    cur_termios_.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    cur_termios_.c_oflag &= ~(OPOST);
    cur_termios_.c_cflag |= (CS8);
    cur_termios_.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    cur_termios_.c_cc[VMIN] = 0;
    cur_termios_.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur_termios_) == -1) {
        throw std::runtime_error("tcsetattr cur failed");
    }
}

TerminalEmulator&
TerminalEmulator::get_singleton()
{
    static TerminalEmulator singleton_instance;
    return singleton_instance;
}

void
TerminalEmulator::update_size()
{
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size_) == -1)
        throw std::runtime_error("ioctl failed");
}
void
TerminalEmulator::detach_screen() noexcept
{
    printf("\033[?1049l\0338");
    screen_status_ = ScreenStatus::detached;
}
void
TerminalEmulator::atach_screen() noexcept
{
    printf("\033[7\033[?1049h");
    screen_status_ = ScreenStatus::atached;
}

void
TerminalEmulator::start_tui_mode()
{
    atach_screen();

    start_raw_mode();
    cursor.hide();
}
void
TerminalEmulator::set_text_color(const RGB color) noexcept
{}
void
TerminalEmulator::reset_text_attributes() noexcept
{}
TerminalEmulator::TerminalEmulator()
{
    update_size();
}
void
TerminalEmulator::stop_tui_mode()
{

    cursor.show();
    stop_raw_mode();

    detach_screen();
}

TerminalEmulator::~TerminalEmulator()
{
    if (screen_status_ != ScreenStatus::detached) {
        stop_tui_mode();
    }
}

TerminalEmulator::TermSize
TerminalEmulator::get_size() const noexcept
{
    return size_;
}

TerminalEmulator::Cursor::Status
TerminalEmulator::Cursor::get_status() noexcept
{
    return status_;
}
void
TerminalEmulator::Cursor::hide() noexcept
{
    status_ = Status::hidden;
    printf("%s", "\033[?;25;l");
}
void
TerminalEmulator::Cursor::show() noexcept
{
    status_ = Status::visible;
    printf("%s", "\033[?;25;h");
}

void
TerminalEmulator::Cursor::shift_left() noexcept(allow_noexcept)
{
    --cur_pos_.x_;
    printf("%s", "\033[1D");
}
void
TerminalEmulator::Cursor::shift_right() noexcept(allow_noexcept)
{
    ++cur_pos_.x_;
    printf("%s", "\033[1C");
}
void
TerminalEmulator::Cursor::shift_up() noexcept(allow_noexcept)
{
    --cur_pos_.y_;
    printf("%s", "\033[1A");
}
void
TerminalEmulator::Cursor::shift_down() noexcept(allow_noexcept)
{
    ++cur_pos_.y_;
    printf("%s", "\033[1B");
}
void
TerminalEmulator::Cursor::move(const Coord<int>& pos) noexcept(allow_noexcept)
{
    cur_pos_ = pos;
    printf("\033[%d;%dH", pos.y_, pos.x_);
}
TerminalEmulator::Cursor::PosType
TerminalEmulator::Cursor::get_pos() const noexcept
{

    return cur_pos_;
}
