#pragma once
#include <bitset>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::render::graph {
	class __declspec(novtable) SceneWalkerFilter :
		public InheritBase<SceneWalkerFilter> {
	public:
		using this_type = SceneWalkerFilter;

		using checksum_type = u64;

	public:
		constexpr SceneWalkerFilter() noexcept = default;

		constexpr ~SceneWalkerFilter() noexcept override = default;

	public:
		[[nodiscard]] virtual checksum_type checksum() const noexcept = 0;

	public:
		virtual void update(cref<gfx::scene::SceneView> sceneView_) noexcept = 0;

		[[nodiscard]] virtual bool operator()(const void* node_) const noexcept = 0;
	};
}
