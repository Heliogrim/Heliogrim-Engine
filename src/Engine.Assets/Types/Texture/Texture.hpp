#pragma once
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>
#include <Engine.Resource/Loader/StreamLoader.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../Asset.hpp"

namespace ember::engine::assets {
    class Texture :
        public Asset {
    public:
        template <typename>
        friend class serialization::DataLayout;

        ACCESS_LAYOUT
        ACCESS_STRUCTURE

    public:
        static constexpr asset_type_id typeId { "Texture"_typeId };

    private:
    public:
        Texture();

    protected:
        Texture(cref<asset_guid> guid_);

    public:
        Texture(
            cref<asset_guid> guid_,
            cref<asset_guid> baseImage_,
            mref<Vector<asset_guid>> images_,
            cref<math::uivec3> extent_,
            cref<gfx::TextureFormat> format_,
            cref<u32> mipLevel_,
            cref<gfx::TextureType> type_
        );

    private:
        asset_guid _baseImage;
        Vector<asset_guid> _images;

    public:
        [[nodiscard]] cref<asset_guid> baseImage() const noexcept;

        void setBaseImage(const asset_guid imageGuid_);

        void addSourceImage(const asset_guid imageGuid_);

        [[nodiscard]] bool hasSourceImage(const asset_guid imageGuid_);

        void removeSourceImage(const asset_guid imageGuid_);

    private:
        math::uivec3 _extent;

    public:
        [[nodiscard]] cref<math::uivec3> getExtent() const noexcept;

        void setExtent(cref<math::uivec3> extent_);

    private:
        gfx::TextureFormat _format;

    public:
        [[nodiscard]] gfx::TextureFormat getTextureFormat() const noexcept;

        void setTextureFormat(const gfx::TextureFormat format_);

    private:
        u32 _mipLevel;

    public:
        [[nodiscard]] u32 getMipLevelCount() const noexcept;

        void setMipLevelCount(const u32 levels_);

    private:
        gfx::TextureType _textureType;

    public:
        [[nodiscard]] gfx::TextureType getTextureType() const noexcept;

        void setTextureType(const gfx::TextureType type_);
    };
}

namespace ember::engine::assets {
    template <>
    struct MetaStreamLoaderAsset<::ember::engine::assets::Texture> {
        using type = ::ember::engine::assets::Texture;
        inline static constexpr bool value = true;
    };
}

namespace ember::engine::res {
    template <>
    struct StreamLoaderOptions<::ember::engine::assets::Texture> {
        u32 layer;
        u32 mip;
        math::uivec3 offset;
        math::uivec3 extent;
    };
}
