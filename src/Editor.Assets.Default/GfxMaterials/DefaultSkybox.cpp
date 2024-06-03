#include "DefaultSkybox.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "DefaultSkyboxPrototype.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultSkybox::DefaultSkybox() :
    GfxMaterialAsset(
        clone(DefaultSkybox::unstable_auto_guid()),
        clone(DefaultSkyboxPrototype::unstable_auto_guid())
    ) {}
