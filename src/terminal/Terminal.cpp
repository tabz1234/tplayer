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

    std::optional<struct winsize> size_;

    std::optional<std::pair<int, int>> pos_;

    struct termios orig_termios_;
    struct termios cur_termios_;

    void stop_raw_mode() {
        const auto c_api_ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios_);
        check(c_api_ret != -1, "tcsetattr orig failed");
    }

    void start_raw_mode() {

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

    void detach_screen() noexcept {
        Terminal::write_str("\033[?1049l\0338");
    }
    void atach_screen() noexcept {
        Terminal::write_str("\033[7\033[?1049h");
    }

} // namespace

void Terminal::write_str(std::string_view str) noexcept {
    write(STDOUT_FILENO, str.data(), str.size());
}

void Terminal::write_char(const char ch) noexcept {
    write(STDOUT_FILENO, &ch, 1);
}

void Terminal::redirect_stderr() noexcept {
    stderr_state = false;
    freopen("/dev/null", "a+", stderr);
}
void Terminal::connect_stderr() noexcept {
    stderr_state = true;
    freopen("/dev/tty", "w", stderr);
}
void Terminal::redirect_stdout() noexcept {
    stdout_state = false;
    freopen("/dev/null", "a+", stdout);
}
void Terminal::connect_stdout() noexcept {
    stdout_state = true;
    freopen("/dev/tty", "w", stdout);
}

void Terminal::clear() noexcept {
    Terminal::write_str("\033[2J");
}
template <std::integral IntT, std::integral auto buff_size_>
std::string_view static integer_to_chars(const IntT int_val,
                                         std::span<char, buff_size_> buff_view) {

    const auto [ptr, ec] =
        std::to_chars(buff_view.data(), buff_view.data() + buff_view.size(), int_val);

    return {buff_view.data(), ptr};
}
void Terminal::set_fg_color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept {

    Terminal::write_str("\033[38;2;");

    constexpr auto buff_size = 4;
    std::array<char, buff_size> conv_buff;

    const auto red_str = integer_to_chars<uint8_t, buff_size>(r, conv_buff);
    Terminal::write_str(red_str);
    Terminal::write_char(';');

    const auto green_str = integer_to_chars<uint8_t, buff_size>(g, conv_buff);
    Terminal::write_str(green_str);
    Terminal::write_char(';');

    const auto blue_str = integer_to_chars<uint8_t, buff_size>(b, conv_buff);
    Terminal::write_str(blue_str);
    Terminal::write_char('m');
}
void Terminal::flush() noexcept {
    fflush(stdout);
}
void Terminal::update_size() {

    if (!stdout_state) {
        connect_stdout();
    } // one cant control when sigwinch raises so check is not consumer
      // responsibility

    size_.emplace();

    const auto c_api_ret = ioctl(STDOUT_FILENO, TIOCGWINSZ, &size_.value());
    check(c_api_ret != -1, "ioctl failed");
}
void Terminal::start_tui_mode() {

    atach_screen();

    update_size();
    Cursor::move(1, 1);

    start_raw_mode();
    Cursor::hide();
}
void Terminal::reset_attributes() noexcept {
    Terminal::write_str("\033[0m");
}
void Terminal::stop_tui_mode() {

    detach_screen();

    Cursor::show();
    stop_raw_mode();
}

struct winsize Terminal::get_size() {
    return size_.value();
}

void Terminal::Cursor::move(const int x, const int y) noexcept {

    pos_ = {x, y};

    std::string esq_str;
    esq_str.reserve(50);

    esq_str += "\033[";
    esq_str += std::to_string(y);
    esq_str += ";";
    esq_str += std::to_string(x);
    esq_str += "H";

    write_str(esq_str);
}
void Terminal::Cursor::hide() noexcept {
    write_str("\033[?;25;l");
}
void Terminal::Cursor::show() noexcept {
    write_str("\033[?;25;h");
}

void Terminal::Cursor::shift_left() noexcept {
    auto& [x, y] = pos_.value();
    --x;

    write_str("\033[1D");
}
void Terminal::Cursor::shift_right() noexcept {
    auto& [x, y] = pos_.value();
    ++x;

    write_str("\033[1C");
}
void Terminal::Cursor::shift_down() noexcept {
    auto& [x, y] = pos_.value();
    --y;

    write_str("\033[1B");
}
void Terminal::Cursor::shift_up() noexcept {
    auto& [x, y] = pos_.value();
    ++y;

    write_str("\033[1A");
}
std::pair<int, int> Terminal::Cursor::get_pos() noexcept {
    return pos_.value();
}
