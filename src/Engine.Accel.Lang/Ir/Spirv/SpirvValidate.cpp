#include <spirv-tools/libspirv.hpp>

/**/

#include "SpirvValidate.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

ir::Diagnostics ir::spirv::validate(const std::span<const u32> words_, const StringView module_) {

	Diagnostics diags {};
	auto errors = 0uz;
	const auto text = [&](const char* message_, const std::size_t index_) {
		return "spir-v module '" + String(module_) + "' is invalid at word " + std::to_string(index_) + ": " + message_;
	};

	spvtools::SpirvTools tools { SPV_ENV_VULKAN_1_3 };
	tools.SetMessageConsumer(
		[&](const spv_message_level_t level_, const char*, const spv_position_t& position_, const char* message_) {
			switch (level_) {
				case SPV_MSG_FATAL:
				case SPV_MSG_INTERNAL_ERROR:
				case SPV_MSG_ERROR: {
					++errors;
					diags.emplace_back(Diagnostic::error(codes::asl("internal-spirv-invalid"), text(message_, position_.index)));
					break;
				}
				case SPV_MSG_WARNING: {
					diags.emplace_back(Diagnostic::warning(codes::asl("internal-spirv-invalid"), text(message_, position_.index)));
					break;
				}
				default: break;
			}
		}
	);

	// a failed validation that reported no error must not read as a pass
	if (not tools.Validate(words_.data(), words_.size()) && errors == 0uz) {
		diags.emplace_back(
			Diagnostic::error(
				codes::asl("internal-spirv-invalid"),
				"spir-v module '" + String(module_) + "' is invalid: the validator failed without a message"
			)
		);
	}
	return diags;
}
