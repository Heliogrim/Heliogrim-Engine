#include <spirv-tools/optimizer.hpp>

/**/

#include "SpirvOptimize.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

ir::Diagnostics ir::spirv::optimize(ref<Vector<u32>> words_, const StringView module_) {

	String messages {};
	spvtools::Optimizer optimizer { SPV_ENV_VULKAN_1_3 };
	optimizer.SetMessageConsumer(
		[&](const spv_message_level_t level_, const char*, const spv_position_t&, const char* message_) {
			if (level_ <= SPV_MSG_ERROR) {
				messages += messages.empty() ? ": " : "; ";
				messages += message_;
			}
		}
	);
	optimizer.RegisterPerformancePasses();

	Vector<u32> out {};
	if (not optimizer.Run(words_.data(), words_.size(), &out)) {
		Diagnostics diags {};
		diags.emplace_back(
			Diagnostic::warning(
				codes::asl("internal-spirv-optimize"),
				"spir-v module '" + String(module_) + "' was not optimized" + messages
			)
		);
		return diags;
	}

	words_ = std::move(out);
	return {};
}
