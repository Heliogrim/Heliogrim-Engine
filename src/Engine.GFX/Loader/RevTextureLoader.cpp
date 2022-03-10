#include "RevTextureLoader.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevTextureLoader::RevTextureLoader(cref<sptr<Device>> device_) :
    TextureLoader(device_) {}

RevTextureLoader::result_type RevTextureLoader::operator()(const ptr<assets::Texture> asset_, options_type options_) {
    return nullptr;
}
