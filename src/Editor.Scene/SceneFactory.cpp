#include "SceneFactory.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Scene.Game/GameScene.hpp>
#include <Engine.Scene.Preview/PreviewScene.hpp>

using namespace ::hg::editor;
using namespace ::hg::engine::scene;
using namespace ::hg;

uptr<SceneBase> SceneFactory::createEditorScene() {
	return createGameScene();
}

uptr<SceneBase> SceneFactory::createGameScene() {
	return make_uptr<GameScene>();
}

uptr<SceneBase> SceneFactory::createPreviewScene() {
	return make_uptr<PreviewScene>();
}