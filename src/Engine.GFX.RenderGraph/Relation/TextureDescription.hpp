#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>
#include <Engine.GFX/Texture/__fwd.hpp>

#include "Description.hpp"

namespace hg::engine::gfx::render::graph {
    struct ActiveMipBitMask {
        using mask_type = u16;
        mask_type mask;

        [[nodiscard]] bool active(u8 baseTwoLevel_) const {
            return mask & (0x1 << baseTwoLevel_);
        }

        [[nodiscard]] constexpr static u8 maxMipLevel() noexcept {
            return static_cast<u8>(sizeof(mask) * 8 - 1);
        }

        [[nodiscard]] constexpr bool operator==(const ActiveMipBitMask& other_) const noexcept {
            return mask == other_.mask;
        }

        [[nodiscard]] constexpr bool operator!=(const ActiveMipBitMask& other_) const noexcept {
            return mask != other_.mask;
        }

        [[nodiscard]] constexpr _STD strong_ordering operator<=>(const ActiveMipBitMask& other_) const noexcept {
            return mask <=> other_.mask;
        }
    };

    class TextureDescription final :
        public InheritMeta<TextureDescription, Description> {
    public:
        using this_type = TextureDescription;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        TextureDescription() noexcept;

        TextureDescription(
            mref<DescriptionValue<TextureType>> textureType_,
            mref<DescriptionValue<TextureFormat>> textureFormat_,
            mref<DescriptionValue<u32>> textureLayers_,
            //mref<DescriptionValue<math::uivec3>> textureExtent_,
            mref<DescriptionValue<ActiveMipBitMask>> textureMips_
        ) noexcept;

        ~TextureDescription() override;

    protected:
        [[nodiscard]] bool isValueCompatible(
            const non_owning_rptr<const Description> other_
        ) const noexcept override;

    private:
    public:
        DescriptionValue<TextureType> _textureType;
        DescriptionValue<TextureFormat> _textureFormat;

        DescriptionValue<u32> _textureLayers;
        //DescriptionValue<math::uivec3> _textureExtent;

        DescriptionValue<ActiveMipBitMask> _textureMips;

    public:
        [[nodiscard]] bool isValidObject(const nmpt<Texture> texture_) const noexcept;

        [[nodiscard]] bool isValidObject(const nmpt<TextureView> textureView_) const noexcept;

        [[nodiscard]] bool isValidObject(const nmpt<VirtualTexture> texture_) const noexcept;

        [[nodiscard]] bool isValidObject(const nmpt<VirtualTextureView> textureView_) const noexcept;
    };
}
