#include "TextureResource.hpp"

#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Resource/Loader/StreamLoader.hpp>

using namespace ember::engine::gfx;
using namespace ember;

void TextureResource::streamLoad(const ptr<EmberObject> options_) {

    using stream_loader_type = res::StreamLoader<assets::Texture>;

    const auto streamLoader { _streamLoader.lock() };
    if (streamLoader) {
        auto* cst { static_cast<ptr<stream_loader_type>>(streamLoader.get()) };
        cst->streamLoad(this, static_cast<stream_loader_type::stream_options_type>(static_cast<void* const>(options_)));
    }
}

void TextureResource::streamUnload(const ptr<EmberObject> options_) {

    using stream_loader_type = res::StreamLoader<assets::Texture>;

    const auto streamLoader { _streamLoader.lock() };
    if (streamLoader) {
        auto* cst { static_cast<ptr<stream_loader_type>>(streamLoader.get()) };
        cst->streamUnload(this, static_cast<stream_loader_type::stream_options_type>(static_cast<void* const>(options_)));
    } 
}
