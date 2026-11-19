#include "SpirvEmitter.hpp"

#include "SpirvEmitter.Impl.hpp"
#include "SpirvOptimize.hpp"
#include "SpirvValidate.hpp"

import std;

namespace hg::engine::accel::lang::ir::spirv {
	namespace {
		[[nodiscard]] bool failed(ref<const Diagnostics> diags_) {
			return std::ranges::any_of(diags_, [](ref<const Diagnostic> diag_) { return diag_.severity() == Severity::eError; });
		}
	}

	EmitResult emit(ref<const Module> lowered_, ref<const Layout> layout_, Tier tier_) {
		// a depth-only fragment stage has no author entry and still needs an entry point: synthesize an empty one
		if (std::ranges::none_of(lowered_.functions, [](const auto& fn_) { return fn_.name == "main" && fn_.body.has_value(); })) {
			if (lowered_.entry != EntryKind::eFragment) {
				return EmitResult {
					{}, { Diagnostic::error(codes::asl("invalid-entry-signature"), "stage has no 'main' entry function") }
				};
			}

			Module withMain = lowered_;
			const StmtRef root { static_cast<u32>(withMain.stmts.size()) };
			withMain.stmts.emplace_back(Stmt { .kind = StmtKind::eBlock });
			withMain.functions.emplace_back(FunctionDecl { .name = "main", .body = BodyRef { root } });
			return emit(withMain, layout_, tier_);
		}

		Impl impl { lowered_, layout_, tier_ };
		impl.require(spv::Capability::Shader);
		if (tier_ >= Tier::eDev) {
			const auto file = [&](StringView name_) {
				const auto id = impl.w.id();
				Ops ops { id };
				SpirvWriter::string(ops, name_);
				impl.w.debug(spv::Op::OpString, ops);
				impl.debugFiles.emplace_back(id);
			};
			if (lowered_.sources.empty()) {
				file(lowered_.name);
			}
			for (const auto& source : lowered_.sources) {
				file(source.empty() ? StringView { lowered_.name } : StringView { source });
			}
			impl.w.debug(spv::Op::OpSource, Ops { w32(spv::SourceLanguage::Unknown), 0uL, impl.debugFiles.front() });
		}
		impl.checkDeclarations();
		if (not failed(impl.diagnostics)) {
			impl.reach();
		}
		if (not failed(impl.diagnostics)) {
			impl.declareGlobals();
			impl.declareConstants();
			impl.declareFunctions();
		}
		if (not failed(impl.diagnostics)) {
			impl.bodies();
			impl.entryPoint();
			impl.features();
		}
		if (failed(impl.diagnostics)) {
			return EmitResult { {}, std::move(impl.diagnostics) };
		}

		for (const auto capability : impl.capabilities) {
			impl.w.capability(spv::Op::OpCapability, Ops { w32(capability) });
		}
		for (const auto& extension : impl.extensions) {
			Ops ops {};
			SpirvWriter::string(ops, extension);
			impl.w.extension(spv::Op::OpExtension, ops);
		}
		impl.w.memoryModel(spv::Op::OpMemoryModel, Ops { w32(spv::AddressingModel::Logical), w32(spv::MemoryModel::GLSL450) });
		auto words = impl.w.finish();
		if (impl.w.overlong()) {
			impl.fail(codes::asl("internal-spirv-emit"), lowered_.functions[impl.entry].loc, "an instruction exceeds the 65535-word limit");
			return EmitResult { {}, std::move(impl.diagnostics) };
		}

		// dev keeps the unoptimized module so names, debug lines and validator offsets match the source
		if (tier_ == Tier::eBuild) {
			for (auto& diag : optimize(words, lowered_.name)) {
				diag.setPrimary(spanIn(lowered_, lowered_.functions[impl.entry].loc));
				impl.diagnostics.emplace_back(std::move(diag));
			}
		}
		if (tier_ >= Tier::eDev) {
			for (auto& diag : validate(words, lowered_.name)) {
				diag.setPrimary(spanIn(lowered_, lowered_.functions[impl.entry].loc));
				impl.diagnostics.emplace_back(std::move(diag));
			}
			if (failed(impl.diagnostics)) {
				return EmitResult { {}, std::move(impl.diagnostics) };
			}
		}
		return EmitResult { std::move(words), std::move(impl.diagnostics) };
	}
}
