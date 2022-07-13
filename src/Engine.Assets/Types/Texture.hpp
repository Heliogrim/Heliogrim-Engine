#pragma once
#include <Engine.Common/Collection/Vector.hpp>
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

    public:
        static constexpr asset_type_id typeId { "Texture"_typeId };

    protected:
        Texture(cref<asset_guid> guid_);

    public:
        Texture(cref<asset_guid> guid_, cref<asset_guid> baseImage_, mref<Vector<asset_guid>> images_,
            cref<math::uivec3> extent_, cref<gfx::TextureFormat> format_, cref<u32> mipLevel_,
            cref<gfx::TextureType> type_);

    private:
        asset_guid _baseImage;
        Vector<asset_guid> _images;

    public:
        [[nodiscard]] cref<asset_guid> baseImage() const noexcept;

    private:
        math::uivec3 _extent;

    public:
        [[nodiscard]] cref<math::uivec3> getExtent() const noexcept;

    private:
        gfx::TextureFormat _format;

    public:
        [[nodiscard]] gfx::TextureFormat getTextureFormat() const noexcept;

    private:
        u32 _mipLevel;

    public:
        [[nodiscard]] u32 getMipLevelCount() const noexcept;

    private:
        gfx::TextureType _textureType;

    public:
        [[nodiscard]] gfx::TextureType getTextureType() const noexcept;
    };
}
