#include "SceneFactory.hpp"

import Engine.Common;
import Engine.Scene.Game;
import Engine.Scene.Preview;

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
