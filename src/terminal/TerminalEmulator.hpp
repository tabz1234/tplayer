#ifndef TERMINAL_EMULATOR_HPP
#define TERMINAL_EMULATOR_HPP

extern "C"
{
#include <sys/ioctl.h>
#include <termios.h>
}

#include <type_traits>
#include <utility>

#include "Coord.hpp"
#include "RGB.hpp"

class TerminalEmulator final
{

    class Cursor final
    {
        using PosType = Coord<int>;
        static constexpr auto allow_noexcept = std::is_nothrow_assignable_v<PosType, PosType>;

        enum class Status
        {
            unknown,
            hidden,
            visible,
            blinking

        } status_ = Status::unknown;

        PosType cur_pos_;

      public:
        void hide() noexcept;
        void show() noexcept;

        Status get_status() noexcept;

        void shift_left() noexcept(allow_noexcept);
        void shift_right() noexcept(allow_noexcept);
        void shift_up() noexcept(allow_noexcept);
        void shift_down() noexcept(allow_noexcept);

        void move(const PosType&) noexcept(allow_noexcept);

        PosType get_pos() const noexcept;
    };

    using TermSize = struct winsize;
    using TermiosInfo = struct termios;

    enum class ScreenStatus
    {
        atached,
        detached

    } screen_status_ = ScreenStatus::detached;

  private:
    TermSize size_;

    TermiosInfo orig_termios_;
    TermiosInfo cur_termios_;

    TerminalEmulator();

    void atach_screen() noexcept;
    void detach_screen() noexcept;

    void stop_raw_mode();
    void start_raw_mode();

    ~TerminalEmulator();

  public:
    static TerminalEmulator& get_singleton();

    void set_text_color(const RGB color) noexcept;
    void reset_text_attributes() noexcept;

    void stop_tui_mode();
    void start_tui_mode();

    void update_size();

    TermSize get_size() const noexcept;

    Cursor cursor;
};

#endif
