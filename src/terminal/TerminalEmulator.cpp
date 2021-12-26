#include "TerminalEmulator.hpp"

extern "C"
{
#include <unistd.h>
}

#include <array>
#include <charconv>
#include <concepts>
#include <csignal>
#include <span>

#include "check.hpp"

using namespace std::string_literals;

namespace {

bool stdout_state = true;
bool stderr_state = true;

std::optional<std::pair<int, int>> size_;

struct termios orig_termios_;
struct termios cur_termios_;

void
stop_raw_mode()
{
    const auto c_api_ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios_);
    check(c_api_ret != -1, "tcsetattr orig failed");
}

void
start_raw_mode()
{
    auto c_api_ret = tcgetattr(STDIN_FILENO, &orig_termios_);
    check(c_api_ret != -1, "tcgetattr orig failed");

    cur_termios_ = orig_termios_;

    cur_termios_.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    cur_termios_.c_oflag &= ~(OPOST);
    cur_termios_.c_cflag |= (CS8);
    cur_termios_.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    cur_termios_.c_cc[VMIN] = 0;
    cur_termios_.c_cc[VTIME] = 1;

    c_api_ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &cur_termios_);
    check(c_api_ret != -1, "tcsetattr orig failed");
}

void
detach_screen() noexcept
{
    constexpr std::string_view rmcup = "\033[?1049l\0338";
    write(STDOUT_FILENO, rmcup.data(), rmcup.size());
}
void
atach_screen() noexcept
{
    constexpr std::string_view smcup = "\033[7\033[?1049h";
    write(STDOUT_FILENO, smcup.data(), smcup.size());
}

} // namespace

void
Terminal::turn_off_stderr() noexcept
{
    stderr_state = false;
    freopen("/dev/null", "a+", stderr);
}
void
Terminal::turn_on_stderr() noexcept
{
    stderr_state = true;
    freopen("/dev/tty", "w", stderr);
}
void
Terminal::turn_off_stdout() noexcept
{
    stdout_state = false;
    freopen("/dev/null", "a+", stdout);
}

void
Terminal::turn_on_stdout() noexcept
{
    stdout_state = true;
    freopen("/dev/tty", "w", stdout);
}
void
Terminal::clear() noexcept
{
    constexpr std::string_view clear = "\033[2J";
    write(STDOUT_FILENO, clear.data(), clear.size());
}
template<std::integral IntT, std::integral auto buff_size_>
std::string_view static int_to_chars(const IntT int_val, std::span<char, buff_size_> buff_view)
{

    const auto [ptr, ec] = std::to_chars(buff_view.data(), buff_view.data() + buff_view.size(), int_val);

    return { buff_view.data(), ptr };
}
void
Terminal::set_fg_color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept
{

    constexpr std::string_view esq_header = "\033[38;2;";
    write(STDOUT_FILENO, esq_header.data(), esq_header.size());

    constexpr auto buff_size = 4;
    std::array<char, buff_size> conv_buff;

    const auto red_str = int_to_chars<uint8_t, buff_size>(r, conv_buff);
    write(STDOUT_FILENO, red_str.data(), red_str.size());
    write(STDOUT_FILENO, ";", 1);

    const auto green_str = int_to_chars<uint8_t, buff_size>(g, conv_buff);
    write(STDOUT_FILENO, green_str.data(), green_str.size());
    write(STDOUT_FILENO, ";", 1);

    const auto blue_str = int_to_chars<uint8_t, buff_size>(b, conv_buff);
    write(STDOUT_FILENO, blue_str.data(), blue_str.size());
    write(STDOUT_FILENO, "m", 1);
}
void
Terminal::flush() noexcept
{
    fflush(stdout);
}
void
Terminal::update_size()
{
    if (!stdout_state) {
        turn_on_stdout();
    }

    const auto c_api_ret = ioctl(STDOUT_FILENO, TIOCGWINSZ, &size_);
    check(c_api_ret != -1, "ioctl failed");
}
void
Terminal::start_tui_mode()
{
    atach_screen();

    update_size();
    cursor.move(1, 1);

    start_raw_mode();
    cursor.hide();
}
void
Terminal::reset_attributes() noexcept
{
    constexpr std::string_view reset_str = "\033[0m";
    write(STDOUT_FILENO, reset_str.data(), reset_str.size());
}
void
Terminal::stop_tui_mode()
{

    detach_screen();

    cursor.show();
    stop_raw_mode();
}

std::pair<int, int>
Terminal::get_size()
{
    return size_.value();
}

void
Terminal::Cursor::move(const int x, const int y) noexcept
{
    pos_.emplace(x, y);
    printf("\033[%d;%dH", y, x); // output of this specific sequance by parts doesnt work, no idea why
}
void
Terminal::Cursor::hide() noexcept
{
    constexpr std::string_view hide_str = "\033[?;25;l";
    write(STDOUT_FILENO, hide_str.data(), hide_str.size());
}
void
Terminal::Cursor::show() noexcept
{
    constexpr std::string_view show_str = "\033[?;25;h";
    write(STDOUT_FILENO, show_str.data(), show_str.size());
}

void
Terminal::Cursor::shift_left() noexcept
{
    auto& [x, y] = Terminal::cursor.pos_.value();
    --x;

    constexpr std::string_view shift_str = "\033[1D";
    write(STDOUT_FILENO, shift_str.data(), shift_str.size());
}
void
Terminal::Cursor::shift_right() noexcept
{
    auto& [x, y] = Terminal::cursor.pos_.value();
    ++x;

    constexpr std::string_view shift_str = "\033[1C";
    write(STDOUT_FILENO, shift_str.data(), shift_str.size());
}
void
Terminal::Cursor::shift_down() noexcept
{
    auto& [x, y] = Terminal::cursor.pos_.value();
    --y;

    constexpr std::string_view shift_str = "\033[1B";
    write(STDOUT_FILENO, shift_str.data(), shift_str.size());
}
void
Terminal::Cursor::shift_up() noexcept
{
    auto& [x, y] = Terminal::cursor.pos_.value();
    ++y;

    constexpr std::string_view shift_str = "\033[1A";
    write(STDOUT_FILENO, shift_str.data(), shift_str.size());
}
std::pair<int, int>
Terminal::Cursor::get_pos() const noexcept
{
    return pos_.value();
}
Terminal::Cursor&
Terminal::Cursor::get_singleton()
{
    static Cursor singleton_instance;
    return singleton_instance;
}
