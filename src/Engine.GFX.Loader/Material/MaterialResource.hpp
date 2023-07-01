#pragma once

#include <Engine.Resource/Manage/Resource.hpp>
#include <Engine.GFX.Material/Material.hpp>

namespace hg::engine::gfx {
    typedef resource::Resource<material::Material> MaterialResource;
}

namespace hg::engine::gfx {
    struct MaterialPrototypeWrapper {
        smr<material::MaterialPrototype> instance;
    };

    typedef resource::Resource<MaterialPrototypeWrapper> MaterialPrototypeResource;
}
