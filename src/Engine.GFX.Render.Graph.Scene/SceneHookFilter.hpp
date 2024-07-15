#pragma once
#include <bitset>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::render {
	class RenderSceneSystemModel;
}

namespace hg::engine::render::graph {
	class __declspec(novtable) SceneHookFilter :
		public InheritBase<SceneHookFilter> {
	public:
		using this_type = SceneHookFilter;

		using checksum_type = u64;

	public:
		constexpr SceneHookFilter() noexcept = default;

		constexpr ~SceneHookFilter() noexcept override = default;

	public:
		[[nodiscard]] virtual checksum_type checksum() const noexcept = 0;

	public:
		[[nodiscard]] virtual bool operator()(const ptr<RenderSceneSystemModel> model_) const noexcept = 0;
	};
}
