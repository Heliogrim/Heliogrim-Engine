#pragma once
#include <Engine.Common/Collection/CompactArray.hpp>

#include "Material.hpp"

namespace ember::engine::assets {
    class GfxMaterial :
        public Material {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "GfxMaterial"_typeId };

    protected:
        GfxMaterial(cref<asset_guid> guid_);

    public:
        GfxMaterial(
            cref<asset_guid> guid_,
            cref<asset_guid> albedo_,
            cref<asset_guid> ao_,
            cref<asset_guid> cavity_,
            cref<asset_guid> displacement_,
            cref<asset_guid> gloss_,
            cref<asset_guid> normal_,
            cref<asset_guid> roughness_,
            cref<asset_guid> metalness_,
            cref<asset_guid> specular_,
            cref<asset_guid> alpha_
        );

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
                asset_guid _metalness;
                asset_guid _specular;
                asset_guid _alpha;
            };

            struct {
                asset_guid _textures[10];
            };
        };

    public:
        [[nodiscard]] CompactArray<asset_guid> textures() const noexcept;

        [[nodiscard]] asset_guid diffuse() const noexcept;

        void setDiffuse(const asset_guid textureGuid_);

        [[nodiscard]] asset_guid normal() const noexcept;

        [[nodiscard]] asset_guid roughness() const noexcept;

        [[nodiscard]] asset_guid metalness() const noexcept;

        [[nodiscard]] asset_guid ao() const noexcept;

        [[nodiscard]] asset_guid alpha() const noexcept;
    };
}
