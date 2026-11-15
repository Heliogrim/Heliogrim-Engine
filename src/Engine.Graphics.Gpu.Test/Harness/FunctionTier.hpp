#pragma once

#include <functional>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Graphics/Device/Device.hpp>

#include "Contract.hpp"
#include "Report.hpp"

namespace hg::test::gpu {
	/**
	 * @details One shading function drawn over a float target, one fragment per texel.
	 *
	 * `fragmentAsl` writes `@output (mode = forward, id = "color") f32vec4 color;` and reads its texel from the builtin
	 * `frag_coord`: `frag_coord.xy` is the texel centre (`x + 0.5`, `y + 0.5`, row 0 on top). Inputs derive from the
	 * texel, for example `roughness = frag_coord.x / width`. Includes resolve below `HG_TEST_SHADER_ROOT`.
	 */
	struct FunctionTestDesc {
		StringView name;
		StringView fragmentAsl;
		math::uivec2 extent { 32uL, 32uL };
		std::function<math::fvec4(u32 x_, u32 y_)> expected;
		InputsAt inputs;
		Contract contract { { absEpsilon(defaultAbsEpsilon) } };
	};

	/**
	 * @details Clears to `sentinel`, draws, reads back and reports; the YAML report is always written, the raster stored on pass.
	 */
	[[nodiscard]] Report runFunction(ref<const sptr<engine::graphics::Device>> device_, ref<const FunctionTestDesc> desc_);
}
