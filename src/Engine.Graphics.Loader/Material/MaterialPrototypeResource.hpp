#pragma once

#include <Engine.Graphics.Material/Material.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::graphics {
    struct MaterialPrototypeWrapper {
        smr<material::MaterialPrototype> instance;
    };

    typedef resource::Resource<MaterialPrototypeWrapper> MaterialPrototypeResource;
}
