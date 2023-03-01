#pragma once
#include "Material.hpp"

namespace hg::engine::assets {
    class SfxMaterial :
        public Material {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "SfxMaterial"_typeId };
    };
}
