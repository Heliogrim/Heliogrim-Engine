#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Level/Level.hpp>

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
