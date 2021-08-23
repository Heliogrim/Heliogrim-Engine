#include "TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

ptr<TextureFactory> TextureFactory::_instance = nullptr;

void TextureFactory::destroy() {
    delete _instance;
    _instance = nullptr;
}

TextureFactory::TextureFactory(cref<sptr<Device>> device_) :
    _device(device_) {}

TextureFactory::~TextureFactory() noexcept = default;

cref<sptr<Device>> TextureFactory::device() const noexcept {
    return _device;
}
