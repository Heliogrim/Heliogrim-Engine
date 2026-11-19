#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
	class macro_novtable SceneWalkerFilter :
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
		virtual void update(cref<graphics::scene::SceneView> sceneView_) noexcept = 0;

		[[nodiscard]] virtual bool operator()(const void* node_) const noexcept = 0;
	};
}
