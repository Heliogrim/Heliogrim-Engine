#pragma once

#include <Engine.Common/Collection/List.hpp>

#include "Data.hpp"
#include "LevelLayout.hpp"

namespace ember::engine::assets {
    class Level :
        public Data<LevelLayout> {
    public:
        friend class LevelLayout;

    protected:
        Level(cref<asset_guid> guid_);

    private:
        vector<asset_guid> _entities;
    };
}
