#include "UIBase.hpp"

#include <array>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Graphics.Render.Subpass/Impl/__tmp_helper.hpp>
#include <Engine.Graphics/Geometry/UIVertex.hpp>
#include <Engine.Graphics/vkinc.hpp>

using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg::engine::accel::lang;
using namespace hg;

smr<const AccelerationEffect> engine::render::makeUiBaseEffect() {

	auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
	auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);

	/**/

	const std::array vertexOffsets {
		std::pair<String, u32> { "vertex-position", static_cast<u32>(offsetof(graphics::uivertex, position)) },
		std::pair<String, u32> { "vertex-color", static_cast<u32>(offsetof(graphics::uivertex, color)) },
		std::pair<String, u32> { "vertex-uv", static_cast<u32>(offsetof(graphics::uivertex, uvm)) },
		std::pair<String, u32> { "vertex-layer", static_cast<u32>(offsetof(graphics::uivertex, layer)) }
	};

	// the colour and layer are stored as 8 bit and normalized on fetch, so the shader reads floats
	const std::array vertexFormats {
		std::pair<String, s32> { "vertex-color", static_cast<s32>(vk::Format::eR8G8B8A8Unorm) },
		std::pair<String, s32> { "vertex-layer", static_cast<s32>(vk::Format::eR8Unorm) }
	};

	vertexStage->setIntermediate(
		makeIrIntermediate(R"(ui/ui.vs.asl)", lang::ir::EntryKind::eVertex, vertexOffsets, vertexFormats)
	);
	fragmentStage->setIntermediate(makeIrIntermediate(R"(ui/ui.fs.asl)", lang::ir::EntryKind::eFragment));

	/**/

	Guid guid { static_cast<u32>("__Test__Proxy"_typeId.data), 0, 0, "UIBaseEffect"_typeId.data };

	return make_smr<AccelerationEffect>(
		std::move(guid),
		"test-ui-base-effect",
		std::vector { std::move(vertexStage), std::move(fragmentStage) }
	);
}
