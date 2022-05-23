#pragma once

#include <Engine.Common/Collection/List.hpp>

#include "Data.hpp"
#include "LevelLayout.hpp"

namespace ember::engine::assets {
    class Level :
        public Data<LevelLayout> {
    public:
        friend class LevelLayout;

    public:
        inline static const asset_type_id typeId { "Level"_typeId };

    protected:
        Level(cref<asset_guid> guid_);

    private:
        Vector<asset_guid> _entities;
    };
}
