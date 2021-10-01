#pragma once
#include <Engine.Common/Collection/List.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>

#include "Data.hpp"
#include "TextureLayout.hpp"

namespace ember::engine::assets {

    class Texture :
        public Data<TextureLayout> {
    public:
        friend class TextureLayout;

    protected:
        Texture(cref<asset_guid> guid_);

        Texture(cref<asset_guid> guid_, cref<asset_guid> baseImage_, mref<vector<asset_guid>> images_,
            cref<math::uivec3> extent_, cref<gfx::TextureFormat> format_, cref<u32> mipLevel_,
            cref<gfx::TextureType> type_);

    private:
        asset_guid _baseImage;
        vector<asset_guid> _images;

    private:
        math::uivec3 _extent;

    private:
        gfx::TextureFormat _format;

    private:
        u32 _mipLevel;

    private:
        gfx::TextureType _type;
    };
}
