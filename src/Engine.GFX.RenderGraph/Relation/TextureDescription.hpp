#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>

#include "Description.hpp"

namespace hg::engine::gfx::render::graph {
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
            mref<DescriptionValue<Vector<u32>>> textureMips_
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

        DescriptionValue<Vector<u32>> _textureMips;
    };
}
