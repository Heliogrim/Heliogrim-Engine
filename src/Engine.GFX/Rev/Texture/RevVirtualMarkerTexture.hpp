#pragma once
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/Texture/Texture.hpp>

namespace ember::engine::gfx {

    class RevVirtualMarkerTexture final {
    public:
        RevVirtualMarkerTexture() = default;

        ~RevVirtualMarkerTexture();

    public:
        void setup();

        void destroy();

    private:
        math::uivec3 _extent;
        math::uivec3 _tileExtent;

        Vector<u32> _tilesPerLayer;

    public:
        [[nodiscard]] u32 tileCount() const noexcept;

        [[nodiscard]] u32 tileCount(const u32 level_) const noexcept;

        // s64 < 0 :: unsupported format or tiling
        [[nodiscard]] s64 estimateTileDataSize(const TextureFormat format_) const noexcept;

        // s32 < 0 :: tile out of bound
        [[nodiscard]] s32 tileIndex(const math::uivec3 uvw_, const u32 level_) const noexcept;

        // pair.first < 0 :: tile index out of bound
        [[nodiscard]] _STD pair<s32, math::uivec3> tileFromIndex(const u32 index_) const noexcept;

    private:
        math::uivec2 _supportedMips;

    public:
        [[nodiscard]] bool isSupportedMip(const u32 level_) const noexcept;

    private:
        Texture _texture;
    };

}
