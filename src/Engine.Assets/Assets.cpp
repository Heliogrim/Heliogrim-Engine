#include "Assets.hpp"

#include <Engine.Assets.System/AssetRegistry.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg::engine::assets;
using namespace hg::engine;
using namespace hg;

Assets::Assets(ref<Engine> engine_) :
	CoreModule(engine_),
	_registry(nullptr) {}

Assets::~Assets() {
	destroy();
}

void Assets::setup() {
	_registry = make_uptr<AssetRegistry>();
	static_cast<ptr<AssetRegistry>>(_registry.get())->setup();
}

void Assets::start() {}

void Assets::stop() {}

void Assets::destroy() {
	if (_registry) {
		static_cast<ptr<AssetRegistry>>(_registry.get())->tidy();
		_registry.reset();
	}
}

nmpt<IAssetRegistry> Assets::getRegistry() const noexcept {
	return _registry.get();
}
