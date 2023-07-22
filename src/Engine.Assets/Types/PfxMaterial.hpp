#pragma once
#include "Material.hpp"

namespace hg::engine::assets {
    class PfxMaterial :
        public InheritMeta<PfxMaterial, Material> {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "PfxMaterial"_typeId };
    };
}
