#pragma once

#include <system_error>

namespace tplayer {
    std::error_code prepare_to_write_audio_frame(const unsigned long id) noexcept;
} // namespace tplayer
