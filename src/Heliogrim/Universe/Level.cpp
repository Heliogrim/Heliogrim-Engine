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

bool Level::addActor(ptr<Actor> actor_) {
	::hg::todo_panic();
}

bool Level::removeActor(ptr<Actor> actor_) {
	::hg::todo_panic();
}

Future<ptr<Level>> hg::CreateLevel() noexcept {
	::hg::todo_panic();
}

Vector<ptr<Level>> hg::GetLevels() noexcept {
	::hg::todo_panic();
}

ptr<Level> hg::GetLevelAt(Vector3 location_) noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<ptr<Level>> level_) noexcept {
	::hg::todo_panic();
}
