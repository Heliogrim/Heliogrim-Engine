#include "Image.hpp"

#if TRUE
#include <Engine.GFX/Resource/TextureResource.hpp>
#endif

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
    }),
    _imageResource(nullptr),
    _imageView(nullptr) {}

Image::~Image() {

    // Warning: Workaround which might cause an crash or error
    if (_imageResource) {
        _imageResource = nullptr;
        _imageView = nullptr;
    }

}
