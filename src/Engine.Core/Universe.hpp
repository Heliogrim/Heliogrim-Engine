#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::scene {
	class SceneBase;
}

namespace hg::engine::core {
	class Universe {
	public:
		using this_type = Universe;

	public:
		explicit Universe(mref<uptr<scene::SceneBase>> scene_);

		Universe(cref<this_type>) = delete;

		Universe(mref<this_type>) noexcept = delete;

		~Universe();

	private:
		uptr<scene::SceneBase> _scene;

	public:
		[[nodiscard]] nmpt<scene::SceneBase> getScene() const noexcept;
	};
}
