#include "GameScene.hpp"

using namespace hg::engine::scene;
using namespace hg;

void GameScene::broadcast(const SceneBroadcastFlags flags_) {
	commitChanges<>();
	Scene::broadcast(flags_);
}

nmpt<IComponentRegisterContext> GameScene::registerContext() noexcept {
	return this;
}