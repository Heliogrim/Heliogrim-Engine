#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Editor.Action;
import Engine.Assets.Type;
import Engine.Level;
#endif

namespace hg::editor::scene::actions {
	class SaveLevel :
		public DelegateAction {
	public:
		SaveLevel(mref<Arci<engine::core::Level>> level_);

		~SaveLevel() override;

	private:
		Arci<engine::core::Level> _level;
		Arci<engine::assets::LevelAsset> _levelAsset;
	};
}
