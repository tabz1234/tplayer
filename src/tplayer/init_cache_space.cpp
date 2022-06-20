#include "init_cache_space.hpp"

#include <filesystem>

namespace tplayer {
    void init_cache_space() noexcept
    {
        std::error_code errc;

        const auto tmp_dir = std::filesystem::temp_directory_path();

        std::filesystem::remove(tmp_dir / "tplayer", errc);

        std::filesystem::create_directory(tmp_dir / "tplayer", errc);
        std::filesystem::create_directory(tmp_dir / "tplayer" / "video", errc);
        std::filesystem::create_directory(tmp_dir / "tplayer" / "audio", errc);

        std::filesystem::current_path(tmp_dir / "tplayer");
    }
} // namespace tplayer
