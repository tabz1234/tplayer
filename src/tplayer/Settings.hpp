#pragma once

#include "DrawMethod.hpp"
#include "HWAccelDevice.hpp"

#include <string_view>
#include <vector>

namespace tplayer {
    struct Settings final {

        DrawMethod draw_method = DrawMethod::straight;

        HWAccelDevice hw_dev = HWAccelDevice::NONE;

        std::vector<std::string_view> files_to_process;

        bool interlaced_presentation = true;

        Settings() noexcept;
    };
} // namespace tplayer
