#include "Assets.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets.System/AssetRegistry.hpp>

#include "AssetFactory.hpp"

using namespace hg::engine::assets;
using namespace hg::engine;
using namespace hg;

Assets::Assets(const non_owning_rptr<Engine> engine_) :
    CoreModule(engine_),
    _registry(nullptr),
    _factory(nullptr) {}

Assets::~Assets() {
    destroy();
}

void Assets::setup() {
    _registry = make_uptr<AssetRegistry>();
    static_cast<ptr<AssetRegistry>>(_registry.get())->setup();

    _factory = make_uptr<AssetFactory>(_registry.get(), nullptr);
}

void Assets::start() {}

void Assets::stop() {}

void Assets::destroy() {
    _factory.reset();

    if (_registry) {
        static_cast<ptr<AssetRegistry>>(_registry.get())->tidy();
        _registry.reset();
    }
}

const non_owning_rptr<IAssetRegistry> Assets::getRegistry() const noexcept {
    return _registry.get();
}

const non_owning_rptr<AssetFactory> Assets::getFactory() const noexcept {
    return _factory.get();
}
