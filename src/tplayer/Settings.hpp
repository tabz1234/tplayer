#pragma once

#include "DrawMethod.hpp"
#include "HWAccelDevice.hpp"

#include <string_view>
#include <vector>

namespace tplayer {
    struct Settings final {

        DrawMethod draw_method;
        HWAccelDevice hw_dev;
        std::vector<std::string_view> files_to_process;

        Settings() noexcept;
    };
} // namespace tplayer
