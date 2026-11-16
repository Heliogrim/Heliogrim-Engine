#include "SpirvEmitter.Impl.hpp"

#include <algorithm>
#include <map>

#include "../Interface.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	void Impl::reach() {
		valueTypes.assign(mod.types.size(), 0uL);
		globals.assign(mod.bindings.size(), GlobalVar {});
		constants.assign(mod.constants.size(), 0uL);
		functions.assign(mod.functions.size(), FunctionInfo {});
		reachedFunctions.assign(mod.functions.size(), false);
		reachedBindings.assign(mod.bindings.size(), false);
		reachedConstants.assign(mod.constants.size(), false);

		for (u32 i = 0uL; i < mod.functions.size(); ++i) {
			if (mod.functions[i].name == "main" && mod.functions[i].body.has_value()) {
				entry = i;
			}
		}
		if (entry == ~0uL) {
			fail(codes::asl("internal-spirv-emit"), {}, "no `main` function with a body");
			return;
		}

		Vector<u32> fnWork { entry };
		Vector<u32> stmtWork {};
		Vector<u32> exprWork {};
		reachedFunctions[entry] = true;
		const auto pushExpr = [&](ExprRef ref_) {
			if (ref_.index < mod.exprs.size()) {
				exprWork.emplace_back(ref_.index);
			}
		};

		while (not fnWork.empty() || not stmtWork.empty() || not exprWork.empty()) {
			if (not exprWork.empty()) {
				const auto& expr = mod.exprs[exprWork.back()];
				exprWork.pop_back();
				for (const auto operand : expr.operands) {
					pushExpr(operand);
				}
				if (not expr.local && expr.binding.index < mod.bindings.size()) {
					reachedBindings[expr.binding.index] = true;
				}
				if (expr.constant.index < mod.constants.size() && not reachedConstants[expr.constant.index]) {
					reachedConstants[expr.constant.index] = true;
					pushExpr(mod.constants[expr.constant.index].value);
				}
				if (expr.function.index < mod.functions.size() && not reachedFunctions[expr.function.index]) {
					reachedFunctions[expr.function.index] = true;
					fnWork.emplace_back(expr.function.index);
				}
				continue;
			}
			if (not stmtWork.empty()) {
				const auto& stmt = mod.stmts[stmtWork.back()];
				stmtWork.pop_back();
				pushExpr(stmt.lhs);
				pushExpr(stmt.rhs);
				for (const auto child : stmt.children) {
					if (child.index < mod.stmts.size()) {
						stmtWork.emplace_back(child.index);
					}
				}
				continue;
			}
			const auto& fn = mod.functions[fnWork.back()];
			fnWork.pop_back();
			if (fn.body.has_value() && fn.body->root.index < mod.stmts.size()) {
				stmtWork.emplace_back(fn.body->root.index);
			}
		}
	}

	u32 Impl::vertexCount(bool input_) {
		const auto& attributes = mod.attributes;
		switch (mod.entry) {
			case EntryKind::eGeometry: {
				switch (attributes.geometryInput) {
					case Primitive::ePoints: return 1uL;
					case Primitive::eLines:
					case Primitive::eLineStrip: return 2uL;
					case Primitive::eLinesAdjacency: return 4uL;
					case Primitive::eTrianglesAdjacency: return 6uL;
					default: return 3uL;
				}
			}
			case EntryKind::eTessCtrl: {
				return input_ ? 32uL : attributes.outputVertices;
			}
			// shortcut: per-vertex tessellation inputs sized to the gl_MaxPatchVertices minimum, size by the pipeline's patch size when known
			default: {
				return 32uL;
			}
		}
	}

	void Impl::declareGlobals() {
		const auto entries = computeInterface(mod);

		// input locations: index among the distinct forward-input ids in sorted order
		Vector<String> inputIds {};
		std::map<String, u32> outputLocations {};
		u32 nextOutput = 0uL;
		for (const auto& entry : entries) {
			if (entry.mode != BindMode::eForward) {
				continue;
			}
			if (entry.input) {
				inputIds.emplace_back(entry.id);
			} else if (not entry.implicit && not outputLocations.contains(entry.id)) {
				outputLocations.emplace(entry.id, nextOutput);
				nextOutput += locationCount(mod.types, entry.type);
			}
		}
		u32 pushBlocks = 0uL;
		std::ranges::sort(inputIds);
		inputIds.erase(std::ranges::unique(inputIds).begin(), inputIds.end());

		const auto decorate = [&](u32 id_, spv::Decoration decoration_, Ops extra_ = {}) {
			Ops operands { id_, w32(decoration_) };
			operands.insert(operands.end(), extra_.begin(), extra_.end());
			w.annotation(spv::Op::OpDecorate, operands);
		};
		const auto descriptor = [&](u32 id_, ref<const BindingDecl> binding_) {
			const auto slot = std::ranges::find(layout.slots, binding_.id, &SlotAssignment::id);
			if (slot == layout.slots.end()) {
				fail(codes::asl("internal-inconsistent-layout"), {}, "binding '" + binding_.id + "' has no descriptor slot");
				return;
			}
			decorate(id_, spv::Decoration::DescriptorSet, Ops { slot->set });
			decorate(id_, spv::Decoration::Binding, Ops { slot->binding });
		};
		const auto access = [&](u32 id_, Access access_) {
			if (access_ == Access::eRead) {
				decorate(id_, spv::Decoration::NonWritable);
			} else if (access_ == Access::eWrite) {
				decorate(id_, spv::Decoration::NonReadable);
			}
		};

		for (u32 i = 0uL; i < mod.bindings.size(); ++i) {
			const auto& binding = mod.bindings[i];
			if (not reachedBindings[i] || binding.implicit) {
				// implicit (builtin) variables: Task 7
				continue;
			}
			auto& global = globals[i];
			global.id = w.id();

			if (binding.bindClass == BindClass::eWorkgroup) {
				global.storage = spv::StorageClass::Workgroup;
				global.pointee = type(binding.type);
			} else if (binding.mode == BindMode::eForward) {
				global.storage = binding.input ? spv::StorageClass::Input : spv::StorageClass::Output;
				global.pointee = type(binding.type);
				const bool perVertex = binding.arrayed || (mod.entry == EntryKind::eTessCtrl && not binding.input && not binding.patch);
				if (perVertex) {
					global.arrayedCount = vertexCount(binding.input);
					global.pointee = w.type(spv::Op::OpTypeArray, Ops { global.pointee, u32Constant(global.arrayedCount) });
				}

				u32 location = 0uL;
				if (binding.input) {
					const auto index = static_cast<u32>(std::ranges::lower_bound(inputIds, binding.id) - inputIds.begin());
					if (index >= layout.inputLocations.size()) {
						fail(codes::asl("internal-inconsistent-layout"), {}, "input '" + binding.id + "' has no location");
						continue;
					}
					location = layout.inputLocations[index];
				} else {
					location = outputLocations[binding.id];
				}
				decorate(global.id, spv::Decoration::Location, Ops { location });

				// as GLSL: fragment inputs of non-float scalar or vector type; arrays stripped, Vulkan needs Flat on integer arrays too
				auto inner = binding.type;
				while (inner.index < mod.types.size() && mod.types[inner.index].kind == TypeKind::eArray) {
					inner = mod.types[inner.index].element;
				}
				if (mod.entry == EntryKind::eFragment && binding.input && inner.index < mod.types.size()) {
					const auto& desc = mod.types[inner.index];
					if ((desc.kind == TypeKind::eScalar || desc.kind == TypeKind::eVector) && desc.scalar != ScalarKind::eFloat) {
						decorate(global.id, spv::Decoration::Flat);
					}
				}
				if (binding.patch) {
					decorate(global.id, spv::Decoration::Patch);
				}
			} else {
				switch (binding.bindClass) {
					case BindClass::eUniformBuffer: {
						if (binding.packing == Packing::eStd430) {
							fail(codes::asl("internal-spirv-emit"), {}, "uniform buffer '" + binding.id + "' with std430 packing is not supported");
							continue;
						}
						global.storage = spv::StorageClass::Uniform;
						global.explicitLayout = true;
						global.std140 = true;
						global.pointee = block(binding.type, global.std140, 0uL);
						descriptor(global.id, binding);
						break;
					}
					case BindClass::eStorageBuffer: {
						global.storage = spv::StorageClass::StorageBuffer;
						global.explicitLayout = true;
						global.std140 = binding.packing == Packing::eStd140;
						global.pointee = block(binding.type, global.std140, 0uL);
						descriptor(global.id, binding);
						// as GLSL: read-only only for inputs, an output with default access stays writable
						if (binding.access == Access::eWrite || binding.input) {
							access(global.id, binding.access);
						}
						break;
					}
					case BindClass::eTexture:
					case BindClass::eSampler:
					case BindClass::eStorageImage: {
						global.storage = spv::StorageClass::UniformConstant;
						global.pointee = type(binding.type);
						descriptor(global.id, binding);
						if (binding.bindClass == BindClass::eStorageImage) {
							access(global.id, binding.access);
						}
						break;
					}
					case BindClass::ePushConstant: {
						const auto range = std::ranges::find(layout.push, binding.id, &PushRange::id);
						if (range == layout.push.end()) {
							fail(codes::asl("internal-inconsistent-layout"), {}, "push binding '" + binding.id + "' has no range");
							continue;
						}
						if (++pushBlocks > 1uL) {
							fail(codes::asl("too-many-push-constant-blocks"), {}, "a stage has at most one push constant block, but the module declares more");
							continue;
						}
						global.storage = spv::StorageClass::PushConstant;
						global.explicitLayout = true;
						global.pointee = block(binding.type, false, range->offset);
						break;
					}
					default: {
						fail(codes::asl("internal-spirv-emit"), {}, "binding '" + binding.id + "' has no class");
						continue;
					}
				}
				global.wrapped = global.explicitLayout && binding.type.index < mod.types.size() &&
					mod.types[binding.type.index].kind != TypeKind::eStruct;
			}

			w.global(spv::Op::OpVariable, Ops { pointer(global.storage, global.pointee), global.id, w32(global.storage) });
			interface.emplace_back(global.id);
			nameOf(global.id, binding.localName.empty() ? StringView { binding.id } : StringView { binding.localName });
		}
	}

	void Impl::declareConstants() {
		for (u32 i = 0uL; i < mod.constants.size(); ++i) {
			if (not reachedConstants[i]) {
				continue;
			}
			const auto& constant = mod.constants[i];
			if (constant.value.index >= mod.exprs.size() || mod.exprs[constant.value.index].kind != ExprKind::eLiteral) {
				fail(codes::asl("internal-spirv-emit"), constant.loc, "constant '" + constant.name + "' is not a literal");
				continue;
			}
			const auto& value = mod.exprs[constant.value.index].literal;
			if (not constant.specId.has_value()) {
				// no name: the literal is deduplicated, another constant may share it
				constants[i] = literal(constant.type, value);
				continue;
			}

			const auto spec = std::ranges::find(layout.specConstants, *constant.specId, &SpecConstant::id);
			if (spec == layout.specConstants.end()) {
				fail(codes::asl("internal-inconsistent-layout"), constant.loc, "spec constant '" + constant.name + "' has no id");
				continue;
			}
			const auto typeId = type(constant.type);
			const auto& desc = mod.types[constant.type.index];
			if (desc.kind == TypeKind::eScalar && desc.scalar == ScalarKind::eBool) {
				const bool truth = std::visit([](auto value_) { return static_cast<bool>(value_); }, value.value);
				constants[i] = w.constant(truth ? spv::Op::OpSpecConstantTrue : spv::Op::OpSpecConstantFalse, typeId, Ops {}, true);
			} else {
				Ops words {};
				if (not literalWords(constant.type, value, words)) {
					continue;
				}
				constants[i] = w.constant(spv::Op::OpSpecConstant, typeId, words, true);
			}
			w.annotation(spv::Op::OpDecorate, Ops { constants[i], w32(spv::Decoration::SpecId), spec->constantId });
			nameOf(constants[i], constant.name);
		}
	}

	void Impl::declareFunctions() {
		for (u32 i = 0uL; i < mod.functions.size(); ++i) {
			if (not reachedFunctions[i]) {
				continue;
			}
			const auto& fn = mod.functions[i];
			auto& info = functions[i];
			info.ret = fn.ret.index == ~0uL ? voidType() : type(fn.ret);
			Ops signature { info.ret };
			for (const auto& param : fn.params) {
				auto paramType = type(param.type);
				if (param.kind != ParamKind::eValue) {
					paramType = pointer(spv::StorageClass::Function, paramType);
				}
				info.paramTypes.emplace_back(paramType);
				info.params.emplace_back(w.id());
				signature.emplace_back(paramType);
			}
			info.type = w.type(spv::Op::OpTypeFunction, signature);
			info.id = w.id();
			if (fn.origin.empty()) {
				nameOf(info.id, fn.name);
			}
			// user names only: specialisations and generated (`__`) params stay unnamed
			if (fn.origin.empty()) {
				for (u32 p = 0uL; p < fn.params.size(); ++p) {
					if (StringView { fn.params[p].name }.contains("__")) {
						continue;
					}
					nameOf(info.params[p], fn.params[p].name);
				}
			}
		}
	}

	void Impl::entryPoint() {
		const auto main = functions[entry].id;
		const auto mode = [&](spv::ExecutionMode mode_, Ops extra_ = {}) {
			Ops operands { main, w32(mode_) };
			operands.insert(operands.end(), extra_.begin(), extra_.end());
			w.executionMode(spv::Op::OpExecutionMode, operands);
		};
		const auto& attributes = mod.attributes;

		auto model = spv::ExecutionModel::Fragment;
		switch (mod.entry) {
			case EntryKind::eVertex: {
				model = spv::ExecutionModel::Vertex;
				break;
			}
			case EntryKind::eTessCtrl: {
				model = spv::ExecutionModel::TessellationControl;
				require(spv::Capability::Tessellation);
				mode(spv::ExecutionMode::OutputVertices, Ops { attributes.outputVertices });
				break;
			}
			case EntryKind::eTessEval: {
				model = spv::ExecutionModel::TessellationEvaluation;
				require(spv::Capability::Tessellation);
				mode(
					attributes.tessDomain == TessDomain::eTriangles ? spv::ExecutionMode::Triangles :
					attributes.tessDomain == TessDomain::eQuads ? spv::ExecutionMode::Quads : spv::ExecutionMode::Isolines
				);
				mode(
					attributes.tessSpacing == TessSpacing::eEqual ? spv::ExecutionMode::SpacingEqual :
					attributes.tessSpacing == TessSpacing::eFractionalEven ? spv::ExecutionMode::SpacingFractionalEven :
					spv::ExecutionMode::SpacingFractionalOdd
				);
				mode(attributes.clockwise ? spv::ExecutionMode::VertexOrderCw : spv::ExecutionMode::VertexOrderCcw);
				break;
			}
			case EntryKind::eGeometry: {
				model = spv::ExecutionModel::Geometry;
				require(spv::Capability::Geometry);
				switch (attributes.geometryInput) {
					case Primitive::ePoints: mode(spv::ExecutionMode::InputPoints); break;
					case Primitive::eLines:
					case Primitive::eLineStrip: mode(spv::ExecutionMode::InputLines); break;
					case Primitive::eLinesAdjacency: mode(spv::ExecutionMode::InputLinesAdjacency); break;
					case Primitive::eTrianglesAdjacency: mode(spv::ExecutionMode::InputTrianglesAdjacency); break;
					default: mode(spv::ExecutionMode::Triangles); break;
				}
				switch (attributes.geometryOutput) {
					case Primitive::ePoints: mode(spv::ExecutionMode::OutputPoints); break;
					case Primitive::eLines:
					case Primitive::eLineStrip: mode(spv::ExecutionMode::OutputLineStrip); break;
					default: mode(spv::ExecutionMode::OutputTriangleStrip); break;
				}
				mode(spv::ExecutionMode::OutputVertices, Ops { attributes.maxVertices });
				mode(spv::ExecutionMode::Invocations, Ops { attributes.invocations });
				break;
			}
			case EntryKind::eFragment: {
				model = spv::ExecutionModel::Fragment;
				mode(spv::ExecutionMode::OriginUpperLeft);
				if (attributes.earlyFragmentTests) {
					mode(spv::ExecutionMode::EarlyFragmentTests);
				}
				break;
			}
			case EntryKind::eCompute: {
				model = spv::ExecutionModel::GLCompute;
				mode(spv::ExecutionMode::LocalSize, Ops { attributes.localSize[0], attributes.localSize[1], attributes.localSize[2] });
				break;
			}
		}

		Ops operands { w32(model), main };
		SpirvWriter::string(operands, "main");
		operands.insert(operands.end(), interface.begin(), interface.end());
		w.entryPoint(spv::Op::OpEntryPoint, operands);
	}
}
