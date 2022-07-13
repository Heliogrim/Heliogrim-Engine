#pragma once
#include <Engine.Common/Collection/CompactArray.hpp>

#include "GfxMaterialLayout.hpp"
#include "Material.hpp"

namespace ember::engine::assets {

    class GfxMaterial :
        public Material<GfxMaterialLayout> {
    public:
        friend class GfxMaterialLayout;

    public:
        inline static const asset_type_id typeId { "GfxMaterial"_typeId };

    protected:
        GfxMaterial(cref<asset_guid> guid_);

    public:
        GfxMaterial(cref<asset_guid> guid_, cref<asset_guid> albedo_, cref<asset_guid> ao_, cref<asset_guid> cavity_,
            cref<asset_guid> displacement_, cref<asset_guid> gloss_, cref<asset_guid> normal_,
            cref<asset_guid> roughness_, cref<asset_guid> specular_);

    private:
        union {
            struct {
                asset_guid _albedo;
                asset_guid _ao;
                asset_guid _cavity;
                asset_guid _displacement;
                asset_guid _gloss;
                asset_guid _normal;
                asset_guid _roughness;
                asset_guid _specular;
            };

            struct {
                asset_guid _textures[8];
            };
        };

    public:
        [[nodiscard]] CompactArray<asset_guid> textures() const noexcept;

        [[nodiscard]] asset_guid diffuse() const noexcept;

        [[nodiscard]] asset_guid normal() const noexcept;

        [[nodiscard]] asset_guid roughness() const noexcept;

        [[nodiscard]] asset_guid ao() const noexcept;
    };
}
