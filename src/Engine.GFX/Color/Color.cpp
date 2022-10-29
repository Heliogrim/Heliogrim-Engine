#include "Color.hpp"

using namespace ember::engine;
using namespace ember::math;

color::color() :
    math::fvec4() {}

color::color(const float& r_, const float& g_, const float& b_, const float& a_) :
    math::fvec4(r_, g_, b_, a_) {}

color::color(const fvec3& rgb_, const float a_) :
    math::fvec4(rgb_, a_) {}

color::color(const fvec4& rgba_) :
    math::fvec4(rgba_) {}

fvec3 color::rgb() const {
    return fvec4::xyz();
}

fvec4 color::rgba() const {
    return *static_cast<const fvec4*>(this);
}

fvec3 color::hsl() const {
    // TODO: return types::fvec4::xyz();
    return {};
}

fvec4 color::hsla() const {
    // TODO *static_cast<const types::fvec4*>(this);
    return {};
}

color::operator vec3_t<float>() const {
    return fvec4::xyz();
}

color::operator vec4_t<float>() const {
    return *static_cast<const fvec4*>(this);
}
