#include "vertex.hpp"
#include <cassert>
#include <iostream>

namespace om
{
std::istream& operator>>(std::istream& in, Vertex& vertex)
{
    in >> vertex.position.x;
    in >> vertex.position.y;
    myGlfloat r, g, b, a;
    in >> r;
    in >> g;
    in >> b;
    in >> a;
    vertex.color.set_r(r);
    vertex.color.set_g(g);
    vertex.color.set_b(b);
    vertex.color.set_a(a);
    return in;
}

std::istream& operator>>(std::istream& in, Triangle<Vertex>& triangle)
{
    in >> triangle.v[0];
    in >> triangle.v[1];
    in >> triangle.v[2];
    return in;
}

myGlfloat Color::get_r() const
{
    uint_least32_t r_ = (rgba & 0x000000FF) >> 0;
    return r_ / 255.f;
}
myGlfloat Color::get_g() const
{
    uint_least32_t g_ = (rgba & 0x0000FF00) >> 8;
    return g_ / 255.f;
}
myGlfloat Color::get_b() const
{
    uint_least32_t b_ = (rgba & 0x00FF0000) >> 16;
    return b_ / 255.f;
}
myGlfloat Color::get_a() const
{
    uint_least32_t a_ = (rgba & 0xFF000000) >> 24;
    return a_ / 255.f;
}

void Color::set_r(const myGlfloat r)
{
    uint_least32_t r_ = static_cast<uint_least32_t>(r * 255);
    rgba &= 0xFFFFFF00;
    rgba |= (r_ << 0);
}
void Color::set_g(const myGlfloat g)
{
    uint_least32_t g_ = static_cast<uint_least32_t>(g * 255);
    rgba &= 0xFFFF00FF;
    rgba |= (g_ << 8);
}
void Color::set_b(const myGlfloat b)
{
    uint_least32_t b_ = static_cast<uint_least32_t>(b * 255);
    rgba &= 0xFF00FFFF;
    rgba |= (b_ << 16);
}
void Color::set_a(const myGlfloat a)
{
    uint_least32_t a_ = static_cast<uint_least32_t>(a * 255);
    rgba &= 0x00FFFFFF;
    rgba |= a_ << 24;
}

} // namespace om
