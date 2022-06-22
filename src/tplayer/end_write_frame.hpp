#pragma once

#include <system_error>

namespace tplayer {
    std::error_code end_write_frame() noexcept;
}
