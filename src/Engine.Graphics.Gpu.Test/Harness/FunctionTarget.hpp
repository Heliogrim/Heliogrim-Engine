#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Graphics/Device/Device.hpp>
#include <Engine.Graphics/Texture/Texture.hpp>

#include "Raster.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::test::gpu {
	// A value no shading function under test writes; exactly representable
	inline constexpr static f32 sentinel = -4096.0F;

	/**
	 * @details Float render target the tests clear, draw into and read back; rests in `eColorAttachmentOptimal`.
	 */
	class FunctionTarget {
	public:
		FunctionTarget(ref<const sptr<engine::graphics::Device>> device_, const math::uivec2 extent_);

		FunctionTarget(ref<const FunctionTarget>) = delete;

		~FunctionTarget();

	public:
		ref<FunctionTarget> operator=(ref<const FunctionTarget>) = delete;

	private:
		sptr<engine::graphics::Device> _device;
		math::uivec2 _extent;
		smr<engine::graphics::Texture> _texture;

	public:
		void clear(const math::fvec4 value_);

		[[nodiscard]] Raster read();

		[[nodiscard]] smr<engine::graphics::Texture> texture() const;

		[[nodiscard]] math::uivec2 extent() const;
	};
}
