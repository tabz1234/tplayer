#include "compute_scaling_factor.hpp"

#include <algorithm>

namespace tplayer {
    double compute_scaling_factor(int window_width, int window_height, int frame_width, int frame_height) noexcept
    {
        double ret;

        double width_ratio = frame_width / static_cast<double>(window_width);
        double height_ratio = frame_height / static_cast<double>(window_height);
        ret = std::min(width_ratio, height_ratio);

        return ret;
    }
} // namespace tplayer
