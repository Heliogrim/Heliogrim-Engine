#include "InputServiceInit.hpp"

#include "InputBindingList.hpp"
#include "InputService.hpp"

import Editor.Action;
import Editor.Core;
import Editor.Scene.Actions;
import Engine.Assets;
import Engine.Assets.System;
import Engine.Assets.Type;
import Engine.Core;
import Engine.Diagnostics;
import Engine.Level;

using namespace hg::editor::ui::service;
using namespace hg;

void editor::ui::service::init_input_service(ref<InputService> service_) {

	auto general = make_uptr<InputBindingList>();
	general->addBinding(
		InputBinding {
			.label = "Save", .binding = { 0x40u | 0x80u, 115u }, .condition = nullptr, .invoke = [] -> Result<Arci<Action>, engine::diag::Diagnostic> {

				auto engine = EditorEngine::getEngine();
				auto assets = ::hg::engine::Assets::get();
				auto universe = engine->getPrimaryGameSession()->getUniverseContext().getCurrentUniverse();

				auto level = universe->getRootLevel();
				if (level == nullptr) {
					return engine::diag::Diagnostic::error(editor::codes::edt("no-root-level"), "Cannot save: the universe has no root level.");
				}

				auto asset = assets->getRegistry()->findAssetByGuid(level->assetGuid());
				if (asset == None || not IsType<engine::assets::LevelAsset>(**asset)) {
					return engine::diag::Diagnostic::error(
						editor::codes::edt("level-asset-unregistered"),
						"Cannot save: no level asset is registered for the root level."
					);
				}

				return Expected { Arci<scene::actions::SaveLevel>::create(::hg::move(level)).into<Action>() };
			}
		}
	);

	service_.addInputBindings(general_input_binding_list, ::hg::move(general));
}
