#pragma once

template<typename T>
struct Coord
{
    T x_;
    T y_;

    virtual ~Coord() = default;
};

