#include "Level.hpp"

#include <Engine.Asserts/Todo.hpp>

using namespace hg;

Level::Level() noexcept {
	::hg::todo_panic();
}

Level::Level(cref<Level> other_) noexcept {
	::hg::todo_panic();
}

Level::Level(mref<Level> other_) noexcept {
	::hg::todo_panic();
}

Level::~Level() noexcept {
	::hg::todo_panic();
}

Future<ptr<Level>> hg::CreateLevel() noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<ptr<Level>> level_) noexcept {
	::hg::todo_panic();
}
