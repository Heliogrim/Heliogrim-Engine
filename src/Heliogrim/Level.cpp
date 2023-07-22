#include "Level.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace hg;

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

Future<ptr<Level>> hg::CreateLevel() noexcept {
    throw NotImplementedException {};
}

Future<bool> hg::Destroy(mref<ptr<Level>> level_) noexcept {
    throw NotImplementedException {};
}
