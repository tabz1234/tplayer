#include "init_cache_space.hpp"

#include <filesystem>

#include "../util/fcheck.hpp"

namespace tplayer {
    int init_cache_space() noexcept
    {
        int ret = 0;
        std::error_code errc;

        const auto tmp_dir = std::filesystem::temp_directory_path(errc);
        fcheck(!errc, [&] {
            fprintf(stderr, "failed to find dir for cache ...");
            ret = 1;
        });

        std::filesystem::create_directory(tmp_dir / "tplayer", errc);
        fcheck(!errc, [&] {
            fprintf(stderr, "failed to create tplayer dir ...");
            ret = 1;
        });
        std::filesystem::create_directory(tmp_dir / "tplayer" / "video", errc);
        fcheck(!errc, [&] {
            fprintf(stderr, "failed to create tplayer/video dir ...");
            ret = 1;
        });
        std::filesystem::create_directory(tmp_dir / "tplayer" / "audio", errc);
        fcheck(!errc, [&] {
            fprintf(stderr, "failed to create tplayer/audio dir ...");
            ret = 1;
        });

        std::filesystem::current_path(tmp_dir / "tplayer", errc);
        fcheck(!errc, [&] {
            fprintf(stderr, "failed cd to tplayer dir ...");
            ret = 1;
        });

        return ret;
    }
} // namespace tplayer
