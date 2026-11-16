#include "SpirvEmitter.hpp"

#include "SpirvEmitter.Impl.hpp"
#include "SpirvValidate.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	EmitResult emit(ref<const Module> lowered_, ref<const Layout> layout_, Tier tier_) {
		Impl impl { lowered_, layout_, tier_ };
		impl.require(spv::Capability::Shader);
		impl.reach();
		if (impl.diagnostics.empty()) {
			impl.declareGlobals();
			impl.declareConstants();
			impl.declareFunctions();
		}
		if (impl.diagnostics.empty()) {
			impl.bodies();
			impl.entryPoint();
		}
		if (not impl.diagnostics.empty()) {
			return EmitResult { {}, std::move(impl.diagnostics) };
		}

		for (const auto capability : impl.capabilities) {
			impl.w.capability(spv::Op::OpCapability, Ops { w32(capability) });
		}
		impl.w.memoryModel(spv::Op::OpMemoryModel, Ops { w32(spv::AddressingModel::Logical), w32(spv::MemoryModel::GLSL450) });
		auto words = impl.w.finish();

		if (tier_ >= Tier::eDev) {
			auto errors = validate(words, lowered_.name);
			if (not errors.empty()) {
				for (auto& error : errors) {
					error.setPrimary(spanIn(lowered_, lowered_.functions[impl.entry].loc));
				}
				return EmitResult { {}, std::move(errors) };
			}
		}
		return EmitResult { std::move(words), {} };
	}
}
