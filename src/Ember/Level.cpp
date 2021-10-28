#include "Level.hpp"

using namespace ember;

Level::Level() noexcept {
    throw NotImplementedException {};
}

Level::Level(cref<Level> other_) noexcept {
    throw NotImplementedException {};
}

Level::Level(mref<Level> other_) noexcept {
    throw NotImplementedException {};
}

Level::~Level() noexcept {
    throw NotImplementedException {};
}

ref<Level> Level::operator=(cref<Level> other_) noexcept {
    throw NotImplementedException {};
}

ref<Level> Level::operator=(mref<Level> other_) noexcept {
    throw NotImplementedException {};
}

bool Level::addEntity(cref<Entity> entity_) {
    throw NotImplementedException {};
}

bool Level::removeEntity(cref<Entity> entity_) {
    throw NotImplementedException {};
}

bool level::valid(cref<Level> level_) noexcept {
    throw NotImplementedException {};
}

future<Level> level::create() noexcept {
    throw NotImplementedException {};
}

future<bool> level::destroy(mref<Level> level_) noexcept {
    throw NotImplementedException {};
}
