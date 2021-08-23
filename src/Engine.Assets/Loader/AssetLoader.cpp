#include "AssetLoader.hpp"

#include <Engine.GFX/Loader/ModelLoader.hpp>
#include <Engine.PFX/Loader/ModelLoader.hpp>
#include <Engine.SFX/Loader/ModelLoader.hpp>

using namespace clockwork::__internal::assets::loader;
using namespace clockwork::__internal::resource::types;

AssetLoader::this_type AssetLoader::_this = nullptr;

AssetLoader::AssetLoader() = default;

AssetLoader::const_reference_type AssetLoader::make() {
	if (!_this)
		_this = _STD move(
			_STD unique_ptr<AssetLoader>(new AssetLoader())
		);
	return *(_this.get());
}

AssetLoader::const_reference_type AssetLoader::get() {
	return *(_this.get());
}

AssetLoader::future_type AssetLoader::load(const Url& url_) const {
	const Url graphicUrl;
	const Url physicUrl;
	const Url soundUrl;

	auto graphicModel = gfx::loader::ModelLoader::get().load(graphicUrl);
	auto physicModel = physics::loader::ModelLoader::get().load(physicUrl);
	auto soundModel = sounds::loader::ModelLoader::get().load(soundUrl);

	const future_type::state_type state = nullptr;
	return future_type { state };
}
