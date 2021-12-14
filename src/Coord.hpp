#ifndef COORD_HPP
#define COORD_HPP

template<typename T>
struct Coord
{
    T x_;
    T y_;

    virtual ~Coord() = default;
};

#endif
