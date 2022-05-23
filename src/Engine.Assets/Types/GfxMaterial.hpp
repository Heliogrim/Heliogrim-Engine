#pragma once
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
        asset_guid _albedo;

    private:
        asset_guid _ao;

    private:
        asset_guid _cavity;

    private:
        asset_guid _displacement;

    private:
        asset_guid _gloss;

    private:
        asset_guid _normal;

    private:
        asset_guid _roughness;

    private:
        asset_guid _specular;
    };
}
