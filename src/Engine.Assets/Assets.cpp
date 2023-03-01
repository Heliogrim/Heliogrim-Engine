#include "Assets.hpp"

#include <Engine.Common/Make.hpp>

#include "AssetFactory.hpp"
#include "Database/AssetDatabase.hpp"

using namespace hg::engine::assets;
using namespace hg::engine;
using namespace hg;

Assets::Assets(const non_owning_rptr<Engine> engine_) :
    CoreModule(engine_),
    _database(nullptr),
    _factory(nullptr) {}

Assets::~Assets() {
    destroy();
}

void Assets::setup() {
    _database = make_uptr<AssetDatabase>();
    _factory = make_uptr<AssetFactory>(_database.get());
}

void Assets::start() {}

void Assets::stop() {}

void Assets::destroy() {
    _factory.reset();
    _database.reset();
}

const non_owning_rptr<AssetDatabase> Assets::getDatabase() const noexcept {
    return _database.get();
}

const non_owning_rptr<AssetFactory> Assets::getFactory() const noexcept {
    return _factory.get();
}
