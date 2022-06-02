#include "Settings.hpp"

namespace tplayer {

    Settings::Settings() noexcept
    {
        draw_method = DrawMethod::straight;
        hw_dev = HWAccelDevice::VAAPI;
    }
} // namespace tplayer
