#include "DefaultBrdfMaterial.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Default.hpp"
#include "DefaultBrdfMaterialPrototype.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultBrdfMaterial::DefaultBrdfMaterial() :
	GfxMaterialAsset(
		clone(editor::assets::materials::default_brdf_guid),
		clone(editor::assets::materials::default_brdf_prototype_guid)
	) {}
