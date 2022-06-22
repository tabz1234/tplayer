#include "end_write_frame.hpp"

#include <filesystem>

namespace tplayer {
    std::error_code end_write_frame() noexcept
    {
        std::error_code ret;
        std::filesystem::current_path("../", ret);
        if (ret) [[unlikely]] {
            return ret;
        }
        std::filesystem::current_path("../", ret);

        return ret;
    }
} // namespace tplayer
