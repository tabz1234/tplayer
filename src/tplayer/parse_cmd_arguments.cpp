#include "parse_cmd_arguments.hpp"

#include <filesystem>

static void on_long_option(const char* const opt, tplayer::Settings& app_settings) noexcept;
static void on_short_option(const char opt, tplayer::Settings& app_settings) noexcept;

namespace tplayer {
    void parse_cmd_arguments(const int argc, const char** const argv, Settings& app_settings) noexcept
    {
        for (int i = 1; i < argc; ++i) {
            if (argv[i][0] == '-') {
                if (argv[i][1] == '-') {
                    on_long_option(argv[i], app_settings);
                }
                else {
                    on_short_option(argv[i][1], app_settings);
                }
            }
            else { // file
                if (std::filesystem::is_regular_file(argv[i])) {
                    app_settings.files_to_process.emplace_back(argv[i]);
                }
            }
        }
    }
} // namespace tplayer

static void on_long_option(const char* const opt, tplayer::Settings& app_settings) noexcept
{
}
static void on_short_option(const char opt, tplayer::Settings& app_settings) noexcept
{
}
