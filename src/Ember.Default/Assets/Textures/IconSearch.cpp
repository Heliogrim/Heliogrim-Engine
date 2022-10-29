#include "IconSearch.hpp"
#include "../Images/IconSearch.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::IconSearch::IconSearch() :
    TextureAsset(
        texture::IconSearch::auto_guid(),
        image::IconSearch::auto_guid(),
        {},
        { 144ui32, 144ui32, 1ui32 },
        TextureFormat::eR8G8B8A8Srgb,
        1ui32,
        engine::gfx::TextureType::e2d
    ) { }
