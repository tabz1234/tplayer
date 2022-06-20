#pragma once

extern "C" {
#include <sys/ioctl.h>
}

namespace terminal {
    winsize get_window_size() noexcept;
} // namespace terminal
