#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Level/Level.hpp>

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
}
