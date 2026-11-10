#pragma once

#include <Engine.Resource/Manage/Resource.hpp>
#include <Engine.Graphics.Material/Material.hpp>

namespace hg::engine::gfx {
    struct MaterialPrototypeWrapper {
        smr<material::MaterialPrototype> instance;
    };

    typedef resource::Resource<MaterialPrototypeWrapper> MaterialPrototypeResource;
}
