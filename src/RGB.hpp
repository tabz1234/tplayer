#ifndef RGB_HPP
#define RGB_HPP

#include <cstdint>

struct RGB
{
    using ColorT = uint8_t;

    ColorT r;
    ColorT g;
    ColorT b;
};

#endif
