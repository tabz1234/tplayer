#pragma once

#include "Settings.hpp"

namespace tplayer {
    void parse_cmd_arguments(const int argc, const char** const argv, Settings& app_settings) noexcept;
}
