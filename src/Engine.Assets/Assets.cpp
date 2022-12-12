#include "Assets.hpp"

#include <Engine.Common/Make.hpp>

#include "AssetFactory.hpp"
#include "Database/AssetDatabase.hpp"
#include "Database/AssetSeeder.hpp"

using namespace ember::engine::assets;
using namespace ember::engine;
using namespace ember;

Assets::Assets(const non_owning_rptr<Engine> engine_) :
    CoreModule(engine_),
    _database(nullptr),
    _factory(nullptr),
    _seeder(nullptr) {}

Assets::~Assets() {
    destroy();
}

void Assets::setup() {
    _database = make_uptr<AssetDatabase>();
    _factory = make_uptr<AssetFactory>(_database.get());
    _seeder = make_uptr<AssetSeeder>();
}

void Assets::schedule() {}

void Assets::desync() {}

void Assets::destroy() {
    _seeder.reset();
    _factory.reset();
    _database.reset();
}

const non_owning_rptr<AssetDatabase> Assets::getDatabase() const noexcept {
    return _database.get();
}

const non_owning_rptr<AssetFactory> Assets::getFactory() const noexcept {
    return _factory.get();
}

const non_owning_rptr<AssetSeeder> Assets::getSeeder() const noexcept {
    return _seeder.get();
}
