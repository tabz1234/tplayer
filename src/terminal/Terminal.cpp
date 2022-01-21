#include "Terminal.hpp"

extern "C" {
#include <unistd.h>
}

#include <array>
#include <charconv>
#include <concepts>
#include <csignal>
#include <span>

#include "../check.hpp"

using namespace std::string_literals;

namespace {

    bool stdout_state = true;
    bool stderr_state = true;

    std::optional<struct winsize> screen_size_;

    struct termios orig_termios_;
    struct termios cur_termios_;

    void stop_raw_mode()
    {
        const auto c_api_ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios_);
        check(c_api_ret != -1, "tcsetattr orig failed");
    }

    void start_raw_mode()
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

    void detach_screen() noexcept
    {
        Terminal::write_str("\033[?1049l\0338");
    }
    void atach_screen() noexcept
    {
        Terminal::write_str("\033[7\033[?1049h");
    }

} // namespace

void Terminal::write_str(std::string_view str) noexcept
{
    write(STDOUT_FILENO, str.data(), str.size());
}

void Terminal::write_char(const char ch) noexcept
{
    write(STDOUT_FILENO, &ch, 1);
}

void Terminal::suspend_stderr() noexcept
{
    stderr_state = false;
    freopen("/dev/null", "a+", stderr);
}
void Terminal::enable_stderr() noexcept
{
    stderr_state = true;
    freopen("/dev/tty", "w", stderr);
}
void Terminal::suspend_stdout() noexcept
{
    stdout_state = false;
    freopen("/dev/null", "a+", stdout);
}
void Terminal::enable_stdout() noexcept
{
    stdout_state = true;
    freopen("/dev/tty", "w", stdout);
}

void Terminal::clear() noexcept
{
    Terminal::write_str("\033[2J");
}
template <std::integral IntT, std::integral auto buff_size_>
std::string_view static integer_to_chars(const IntT int_val,
                                         std::array<char, buff_size_>& buff_view)
{

    const auto [ptr, ec] =
        std::to_chars(buff_view.data(), buff_view.data() + buff_view.size(), int_val);

    return {buff_view.data(), ptr};
}
void Terminal::set_fg_color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept
{

    Terminal::write_str("\033[38;2;");

    constexpr auto buff_size = 4;
    std::array<char, buff_size> conv_buff;

    const auto red_str = integer_to_chars(r, conv_buff);
    Terminal::write_str(red_str);
    Terminal::write_char(';');

    const auto green_str = integer_to_chars(g, conv_buff);
    Terminal::write_str(green_str);
    Terminal::write_char(';');

    const auto blue_str = integer_to_chars(b, conv_buff);
    Terminal::write_str(blue_str);
    Terminal::write_char('m');
}
void Terminal::flush() noexcept
{
    fflush(stdout);
}
void Terminal::update_size()
{

    if (!stdout_state) {
        enable_stdout();
    } // one cant control when sigwinch raises so check is not user
      // responsibility

    screen_size_.emplace();

    const auto c_api_ret = ioctl(STDOUT_FILENO, TIOCGWINSZ, &screen_size_.value());
    check(c_api_ret != -1, "ioctl failed");
}
void Terminal::start_tui_mode()
{

    atach_screen();

    update_size();

    start_raw_mode();
    Cursor::hide();
}

void Terminal::stop_tui_mode()
{

    Cursor::show();
    stop_raw_mode();
    detach_screen();

    Terminal::flush();
}

void Terminal::reset_attributes() noexcept
{
    Terminal::write_str("\033[0m");
}
struct winsize Terminal::get_size()
{
    return screen_size_.value();
}

void Terminal::Cursor::move(const int x, const int y) noexcept
{

    std::string esq_str;
    esq_str.reserve(50);

    esq_str += "\033[";
    esq_str += std::to_string(y);
    esq_str += ";";
    esq_str += std::to_string(x);
    esq_str += "H";

    write_str(esq_str);
}
void Terminal::Cursor::hide() noexcept
{
    write_str("\033[?;25;l");
}
void Terminal::Cursor::show() noexcept
{
    write_str("\033[?;25;h");
}

void Terminal::Cursor::shift_left() noexcept
{
    write_str("\033[1D");
}
void Terminal::Cursor::shift_right() noexcept
{
    write_str("\033[1C");
}
void Terminal::Cursor::shift_down() noexcept
{
    write_str("\033[1B");
}
void Terminal::Cursor::shift_up() noexcept
{
    write_str("\033[1A");
}
