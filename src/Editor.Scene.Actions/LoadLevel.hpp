#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Level;
#endif

namespace hg::editor::scene::actions {
	class LoadLevel :
		public DelegateAction {
	public:
		LoadLevel(mref<Arci<engine::assets::LevelAsset>> levelAsset_);

		~LoadLevel() override;

	private:
		Arci<engine::assets::LevelAsset> _levelAsset;
		Arci<engine::core::Level> _level;
	};

	/**
	 * Loads the level data archive of `levelAsset_` through the engine load boundary; errors go to the sink and are returned.
	 */
	[[nodiscard]] Result<Arci<engine::core::Level>, engine::diag::Diagnostics> loadLevelFromAsset(
		_In_ ref<const engine::assets::LevelAsset> levelAsset_,
		_Inout_ ref<engine::diag::DiagnosticSink> sink_
	);
}
