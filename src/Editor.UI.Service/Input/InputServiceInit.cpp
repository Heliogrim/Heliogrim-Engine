#include "InputServiceInit.hpp"

#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Scene.Actions/SaveLevel.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Level/Level.hpp>

#include "InputBindingList.hpp"
#include "InputService.hpp"

using namespace hg::editor::ui::service;
using namespace hg;

void editor::ui::service::init_input_service(ref<InputService> service_) {

	auto general = make_uptr<InputBindingList>();
	general->addBinding(
		InputBinding {
			.label = "Save", .binding = { 0x40u | 0x80u, 115u }, .condition = nullptr, .invoke = [] -> Arci<Action> {

				auto engine = EditorEngine::getEngine();
				auto assets = engine->getAssets();
				auto universe = engine->getPrimaryGameSession()->getUniverseContext().getCurrentUniverse();

				auto level = universe->getRootLevel();

				auto asset = assets->getRegistry()->findAssetByGuid(level->assetGuid());
				if (asset == None || not IsType<engine::assets::LevelAsset>(**asset)) {
					return {};
				}

				return Arci<scene::actions::SaveLevel>::create(::hg::move(level)).into<Action>();
			}
		}
	);

	service_.addInputBindings(general_input_binding_list, ::hg::move(general));
}
