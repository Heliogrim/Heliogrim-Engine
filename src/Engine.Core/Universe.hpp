#pragma once

#include <span>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Engine.Common/Optional.hpp"
#include "Engine.Common/Collection/DenseSet.hpp"
#include "Engine.Common/Managed/Rc.hpp"

namespace hg::engine::core {
	class Level;
}

namespace hg::engine::scene {
	class SceneBase;
}

namespace hg::engine::core {
	class Universe {
	public:
		using this_type = Universe;

	public:
		Universe(
			mref<uptr<scene::SceneBase>> scene_,
			mref<DenseSet<Arci<core::Level>>> levels_
		);

		Universe(cref<this_type>) = delete;

		Universe(mref<this_type>) noexcept = delete;

		~Universe();

	private:
		uptr<scene::SceneBase> _scene;

	public:
		[[nodiscard]] nmpt<scene::SceneBase> getScene() const noexcept;

	private:
		// Question: Should we use Arc<ExclusiveResource< ...<core::Level> >> instead?
		DenseSet<Arci<core::Level>> _levels;
		Opt<Arci<core::Level>> _rootLevel;

	public:
		void commitLevel(ref<core::Level> level_);

		void uncommitLevel(ref<core::Level> level_);

	public:
		void addLevel(mref<Arci<core::Level>> level_);

		[[nodiscard]] std::span<const Arci<core::Level>> getLevels() const noexcept;

		[[nodiscard]] Arci<core::Level> getRootLevel() const noexcept;

		void removeLevel(cref<Arci<core::Level>> level_);
	};
}
