#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "Asset.hpp"

namespace hg::engine::assets {
    class Level :
        public InheritMeta<Level, Asset> {
    public:
        template <typename>
        friend class serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "Level"_typeId };

    protected:
        Level(mref<asset_guid> guid_);

    private:
        Vector<asset_guid> _entities;
    };
}
