#include "DefaultBrdfMaterial.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "DefaultBrdfMaterialPrototype.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultBrdfMaterial::DefaultBrdfMaterial() :
    GfxMaterialAsset(
        DefaultBrdfMaterial::unstable_auto_guid(),
        clone(DefaultBrdfMaterialPrototype::unstable_auto_guid())
    ) {}
