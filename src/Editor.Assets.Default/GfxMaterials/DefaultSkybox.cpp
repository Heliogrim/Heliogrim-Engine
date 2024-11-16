#include "DefaultSkybox.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Default.hpp"
#include "DefaultSkyboxPrototype.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultSkybox::DefaultSkybox() :
	GfxMaterialAsset(
		clone(editor::assets::materials::default_skybox_guid),
		clone(editor::assets::materials::default_skybox_prototype_guid)
	) {}
