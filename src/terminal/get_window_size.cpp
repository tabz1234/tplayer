#include "get_window_size.hpp"

extern "C" {
#include <unistd.h>
}

namespace terminal {
    winsize get_window_size() noexcept
    {
        struct winsize ret;
        ret.ws_col = 0;
        ret.ws_row = 0;
        ret.ws_xpixel = 0;
        ret.ws_ypixel = 0;

        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ret);
        return ret;
    }
} // namespace terminal
