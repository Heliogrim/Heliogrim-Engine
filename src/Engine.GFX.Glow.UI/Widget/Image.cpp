#include "Image.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Image::Image() :
    Widget(),
    _image(),
    _uvs({
        math::vec2 { 0.F, 0.F },
        math::vec2 { 1.F, 0.F },
        math::vec2 { 1.F, 1.F },
        math::vec2 { 0.F, 1.F }
    }) {}
