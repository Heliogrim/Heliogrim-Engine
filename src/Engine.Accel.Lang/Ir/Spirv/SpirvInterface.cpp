#include <algorithm>
#include <limits>
#include <map>
#include <type_traits>
#include <variant>

#include "SpirvEmitter.Impl.hpp"
#include "../Interface.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	void Impl::checkDeclarations() {
		const auto user = [&](ref<const DiagnosticCodeInfo> code_, SourceLoc loc_, String message_) {
			diagnostics.emplace_back(Diagnostic::error(code_, std::move(message_)).at(spanIn(mod, loc_)));
		};
		const auto at = [&](TypeRef ref_) -> const TypeDesc* {
			return ref_.index < mod.types.size() ? &mod.types[ref_.index] : nullptr;
		};

		for (const auto& desc : mod.types) {
			if (desc.kind == TypeKind::eMatrix && desc.scalar != ScalarKind::eFloat) {
				fail(codes::asl("internal-spirv-emit"), {}, "matrices are float only");
			}
		}

		// structs: self containment (depth first, open/done) and one shape per name
		enum class State : u8 { eNew, eOpen, eDone };
		Vector<State> state(mod.types.size(), State::eNew);
		Vector<u32> named {};
		const auto same = [&](this const auto& self_, TypeRef a_, TypeRef b_, u32 depth_) -> bool {
			const auto* a = at(a_);
			const auto* b = at(b_);
			if (a == b) {
				return true;
			}

			// past the depth bound the shapes are taken as equal: a deep identical struct must not conflict
			if (depth_ > 64uL) {
				return true;
			}

			if (a == nullptr || b == nullptr || a->kind != b->kind) {
				return false;
			}

			if (a->kind == TypeKind::eStruct) {
				return a->name == b->name && a->fields.size() == b->fields.size() && std::ranges::equal(
					a->fields,
					b->fields,
					[&](const auto& x_, const auto& y_) { return x_.name == y_.name && self_(x_.type, y_.type, depth_ + 1uL); }
				);
			}
			return a->scalar == b->scalar && a->bits == b->bits && a->n == b->n && a->m == b->m && a->count == b->count &&
				a->name == b->name && a->dim == b->dim && (a->element == b->element || self_(a->element, b->element, depth_ + 1uL));
		};
		const auto use = [&](this const auto& self_, TypeRef ref_, SourceLoc loc_) -> void {
			const auto* desc = at(ref_);
			if (desc == nullptr || state[ref_.index] == State::eDone) {
				return;
			}

			if (state[ref_.index] == State::eOpen) {
				user(codes::asl("recursive-struct"), loc_, "struct '" + desc->name + "' contains itself");
				return;
			}

			state[ref_.index] = State::eOpen;
			if (desc->kind == TypeKind::eArray || desc->kind == TypeKind::eRegion) {
				self_(desc->element, loc_);
			}
			if (desc->kind == TypeKind::eStruct) {
				for (const auto& field : desc->fields) {
					self_(field.type, loc_);
				}
				if (not desc->name.empty()) {
					const auto known = std::ranges::find_if(named, [&](u32 i_) { return mod.types[i_].name == desc->name; });
					if (known == named.end()) {
						named.emplace_back(ref_.index);
					} else if (not same(TypeRef { *known }, ref_, 0uL)) {
						user(codes::asl("conflicting-struct-definitions"), loc_, "conflicting definitions of struct '" + desc->name + "'");
					}
				}
			}
			state[ref_.index] = State::eDone;
		};

		// interface order: inputs first, then id; neighbours with one id and direction are duplicates
		Vector<u32> order(mod.bindings.size());
		for (u32 i = 0uL; i < order.size(); ++i) {
			order[i] = i;
		}
		std::ranges::stable_sort(
			order,
			[&](u32 a_, u32 b_) {
				const auto& a = mod.bindings[a_];
				const auto& b = mod.bindings[b_];
				return std::forward_as_tuple(not a.input, a.id) < std::forward_as_tuple(not b.input, b.id);
			}
		);
		u32 pushBlocks = 0uL;
		for (u32 i = 0uL; i < order.size(); ++i) {
			const auto& binding = mod.bindings[order[i]];
			if (i > 0uL && mod.bindings[order[i - 1uL]].id == binding.id && mod.bindings[order[i - 1uL]].input == binding.input) {
				user(codes::asl("duplicate-binding"), binding.loc, "binding '" + binding.id + "' is declared twice");
				continue;
			}

			use(binding.type, binding.loc);
			if (binding.implicit || binding.mode == BindMode::eForward || binding.bindClass == BindClass::eWorkgroup) {
				continue;
			}

			if (binding.bindClass == BindClass::eNone) {
				user(codes::asl("missing-binding-class"), binding.loc, "binding '" + binding.id + "' has no class");
				continue;
			}

			const auto* desc = at(binding.type);
			const auto opaque = desc != nullptr &&
				(desc->kind == TypeKind::eTexture || desc->kind == TypeKind::eSampler || desc->kind == TypeKind::eImage);
			const auto fits = binding.bindClass == BindClass::eTexture ?
				desc != nullptr && desc->kind == TypeKind::eTexture :
				binding.bindClass == BindClass::eSampler ?
				desc != nullptr && desc->kind == TypeKind::eSampler :
				binding.bindClass == BindClass::eStorageImage ?
				desc != nullptr && desc->kind == TypeKind::eImage :
				desc != nullptr && not opaque;
			if (not fits) {
				user(codes::asl("binding-class-mismatch"), binding.loc, "type of binding '" + binding.id + "' does not fit its class");
				continue;
			}

			if (binding.bindClass != BindClass::ePushConstant) {
				continue;
			}

			if (++pushBlocks == 2uL) {
				const auto total = std::ranges::count_if(
					mod.bindings,
					[](const auto& b_) {
						return b_.bindClass == BindClass::ePushConstant && not b_.implicit && b_.mode != BindMode::eForward;
					}
				);
				user(codes::asl("too-many-push-constant-blocks"),
					binding.loc,
					"a stage has at most one push constant block, but the module declares " + std::to_string(total)
				);
			}
			const auto range = std::ranges::find(layout.push, binding.id, &PushRange::id);
			if (range != layout.push.end() && range->size == 0uL) {
				user(codes::asl("invalid-push-constant-type"), binding.loc, "push constant '" + binding.id + "' cannot be laid out");
			}
		}

		for (const auto& fn : mod.functions) {
			for (const auto& param : fn.params) {
				use(param.type, fn.loc);
			}
			use(fn.ret, fn.loc);
			if (fn.name == "main" && fn.body.has_value() && (not fn.params.empty() || fn.ret.index != TypeRef {}.index)) {
				user(codes::asl("invalid-entry-signature"), fn.loc, "'main' takes no parameters and returns nothing");
			}
		}

		// every other struct the module uses: constants, locals, expressions (nested fields follow from `use`)
		for (const auto& constant : mod.constants) {
			use(constant.type, constant.loc);
		}
		for (const auto& stmt : mod.stmts) {
			use(stmt.type, stmt.loc);
		}
		for (const auto& expr : mod.exprs) {
			use(expr.type, expr.loc);
		}
	}

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

						if (range->size == 0uL) {
							fail(codes::asl("invalid-push-constant-type"), {}, "push constant '" + binding.id + "' cannot be laid out");
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
		using Value = decltype(LiteralValue::value);
		constexpr u32 maxDepth = 64uL;

		// two's complement at the declared width: signed sign-extends, unsigned masks
		const auto wrap = [](const ScalarKind kind_, const u32 bits_, const u64 value_) -> u64 {
			if (bits_ == 0uL || bits_ >= 64uL) {
				return value_;
			}

			const auto shift = 64uL - bits_;
			return kind_ == ScalarKind::eInt ?
				static_cast<u64>(static_cast<s64>(value_ << shift) >> shift) :
				value_ & ((1uLL << bits_) - 1uLL);
		};
		// why the last fold failed, for the diagnostic
		String reason {};
		// a folded value converted to the scalar type `desc_` at its declared width; None for a float outside an integer range
		const auto convert = [&wrap, &reason](ref<const TypeDesc> desc_, const Value value_) -> Opt<Value> {
			if (const auto* const real = std::get_if<f64>(&value_);
				real != nullptr && (desc_.scalar == ScalarKind::eInt || desc_.scalar == ScalarKind::eUInt) && not fitsInteger(*real, desc_.scalar, desc_.bits)) {
				reason = "a float outside the integer range does not convert";
				return None;
			}

			return std::visit(
				[&](const auto v_) -> Value {
					switch (desc_.scalar) {
						case ScalarKind::eBool: return static_cast<bool>(v_);
						case ScalarKind::eInt: return static_cast<s64>(wrap(ScalarKind::eInt, desc_.bits, static_cast<u64>(static_cast<s64>(v_))));
						case ScalarKind::eUInt: return wrap(ScalarKind::eUInt, desc_.bits, static_cast<u64>(v_));
						default: return desc_.bits == 32uL ? static_cast<f64>(static_cast<float>(v_)) : static_cast<f64>(v_);
					}
				},
				value_
			);
		};

		// scalar constant expression folded to a value of the expression's own type; None when it does not fold
		const auto scalarOf = [&](this const auto& self_, const ExprRef ref_, const u32 depth_) -> Opt<Value> {
			if (ref_.index >= mod.exprs.size() || depth_ > maxDepth) {
				return None;
			}

			const auto& e = mod.exprs[ref_.index];
			if (e.type.index >= mod.types.size() || mod.types[e.type.index].kind != TypeKind::eScalar) {
				return None;
			}

			const auto& desc = mod.types[e.type.index];
			const auto as = [&desc, &convert](const Value value_) { return convert(desc, value_); };

			Opt<Value> value {};
			switch (e.kind) {
				case ExprKind::eLiteral: {
					value = e.literal.value;
					break;
				}
				case ExprKind::eNameRef: {
					if (not e.local && e.constant.index < mod.constants.size() && not mod.constants[e.constant.index].specId.has_value()) {
						value = self_(mod.constants[e.constant.index].value, depth_ + 1uL);
					}
					break;
				}
				case ExprKind::eConstruct: {
					if (e.operands.size() == 1uL) {
						value = self_(e.operands.front(), depth_ + 1uL);
					}
					break;
				}
				case ExprKind::eUnary: {
					if (e.op != Op::eNeg || e.operands.size() != 1uL) {
						break;
					}

					const auto operand = self_(e.operands.front(), depth_ + 1uL);
					if (const auto converted = operand.has_value() ? as(*operand) : None; converted.has_value()) {
						value = std::visit(
							[](const auto value_) -> Value {
								using T = decltype(value_);
								if constexpr (std::is_same_v<T, bool>) {
									return value_;
								} else if constexpr (std::is_integral_v<T>) {
									// integers wrap instead of overflowing
									return static_cast<T>(0uLL - static_cast<u64>(value_));
								} else {
									return -value_;
								}
							},
							*converted
						);
					}
					break;
				}
				case ExprKind::eBinary: {
					if (e.operands.size() != 2uL) {
						break;
					}

					const auto lhsValue = self_(e.operands[0], depth_ + 1uL);
					const auto rhsValue = self_(e.operands[1], depth_ + 1uL);
					const auto lhs = lhsValue.has_value() ? as(*lhsValue) : None;
					const auto rhs = rhsValue.has_value() ? as(*rhsValue) : None;
					if (not lhs.has_value() || not rhs.has_value()) {
						break;
					}

					value = std::visit(
						[op = e.op, &reason, minimum = static_cast<s64>(wrap(ScalarKind::eInt, desc.bits, 1uLL << ((desc.bits == 0uL || desc.bits > 64uL ? 64uL : desc.bits) - 1uLL)))](const auto lhs_, const auto rhs_) -> Opt<Value> {
							using T = decltype(lhs_);
							if constexpr (not std::is_same_v<T, decltype(rhs_)> || std::is_same_v<T, bool>) {
								return None;
							} else if constexpr (std::is_floating_point_v<T>) {
								switch (op) {
									case Op::eAdd: return Value { lhs_ + rhs_ };
									case Op::eSub: return Value { lhs_ - rhs_ };
									case Op::eMul: return Value { lhs_ * rhs_ };
									case Op::eDiv: return Value { lhs_ / rhs_ };
									default: return None;
								}
							} else {
								// integers wrap instead of overflowing (the result wraps to the declared width below); division by zero does not fold
								const auto a = static_cast<u64>(lhs_);
								const auto b = static_cast<u64>(rhs_);
								switch (op) {
									case Op::eAdd: return Value { static_cast<T>(a + b) };
									case Op::eSub: return Value { static_cast<T>(a - b) };
									case Op::eMul: return Value { static_cast<T>(a * b) };
									case Op::eDiv:
									case Op::eMod: {
										if (rhs_ == T {}) {
											reason = "division by zero";
											return None;
										}

										if constexpr (std::is_signed_v<T>) {
											if (rhs_ == T { -1 } && lhs_ == minimum) {
												reason = "the most negative value divided by -1 overflows";
												return None;
											}
										}
										// C++ `%` truncates like OpSRem / OpUMod
										return Value { static_cast<T>(op == Op::eDiv ? lhs_ / rhs_ : lhs_ % rhs_) };
									}
									default: return None;
								}
							}
						},
						*lhs,
						*rhs
					);
					break;
				}
				default: break;
			}
			return value.has_value() ? as(*value) : None;
		};

		// components (vector) or columns (matrix) of every folded composite, to flatten nested constructs
		std::map<u32, Ops> parts {};
		// value of every folded scalar, to convert components to the composite's scalar type
		std::map<u32, Value> values {};
		const auto flatten = [&](this const auto& self_, const u32 id_, ref<Ops> out_) -> void {
			if (const auto nested = parts.find(id_); nested != parts.end()) {
				for (const auto part : nested->second) {
					self_(part, out_);
				}
				return;
			}

			out_.emplace_back(id_);
		};
		// constant expression folded to an OpConstant / OpConstantComposite id; 0 when it does not fold
		const auto fold = [&](this const auto& self_, const ExprRef ref_, const u32 depth_) -> u32 {
			if (ref_.index >= mod.exprs.size() || depth_ > maxDepth) {
				return 0uL;
			}

			const auto& e = mod.exprs[ref_.index];
			if (e.type.index >= mod.types.size()) {
				return 0uL;
			}

			const auto& t = mod.types[e.type.index];
			if (t.kind == TypeKind::eScalar) {
				const auto value = scalarOf(ref_, depth_);
				const auto id = value.has_value() ? literal(e.type, LiteralValue { *value }, e.loc) : 0uL;
				if (id != 0uL) {
					values.insert_or_assign(id, *value);
				}
				return id;
			}

			if (e.kind == ExprKind::eNameRef) {
				const bool folds = not e.local && e.constant.index < mod.constants.size() &&
					not mod.constants[e.constant.index].specId.has_value();
				return folds ? self_(mod.constants[e.constant.index].value, depth_ + 1uL) : 0uL;
			}

			if (e.kind != ExprKind::eConstruct) {
				return 0uL;
			}

			Ops operands {};
			Ops flat {};
			for (const auto& operand : e.operands) {
				const auto id = self_(operand, depth_ + 1uL);
				if (id == 0uL) {
					return 0uL;
				}

				operands.emplace_back(id);
				flatten(id, flat);
			}
			// vector and matrix components convert to the composite's scalar type, like the runtime construct
			if (t.kind == TypeKind::eVector || t.kind == TypeKind::eMatrix) {
				const TypeDesc component { .scalar = t.scalar, .bits = t.bits };
				for (auto& id : flat) {
					const auto value = values.find(id);
					if (value == values.end()) {
						return 0uL;
					}

					const auto converted = convert(component, value->second);
					if (not converted.has_value()) {
						return 0uL;
					}

					id = literal(component, LiteralValue { *converted }, e.loc);
					values.insert_or_assign(id, *converted);
				}
			}

			Ops out {};
			switch (t.kind) {
				case TypeKind::eVector: {
					out = flat.size() == 1uL ? Ops(t.n, flat.front()) : flat;
					if (out.size() != t.n) {
						return 0uL;
					}

					break;
				}
				case TypeKind::eMatrix: {
					// shortcut: matrix from scalars, columns or a same-sized matrix only; diagonal and matrix-resize constants do not fold yet, add them when a shader needs one
					if (flat.size() != t.n * t.m) {
						return 0uL;
					}

					const auto column = w.type(spv::Op::OpTypeVector, Ops { scalar(ScalarKind::eFloat, t.bits), t.m });
					for (u32 c = 0uL; c < t.n; ++c) {
						Ops cells(flat.begin() + c * t.m, flat.begin() + (c + 1uL) * t.m);
						const auto id = w.constant(spv::Op::OpConstantComposite, column, cells);
						parts.insert_or_assign(id, std::move(cells));
						out.emplace_back(id);
					}
					break;
				}
				case TypeKind::eArray:
				case TypeKind::eStruct: {
					out = operands;
					if (out.size() != (t.kind == TypeKind::eArray ? t.count : t.fields.size())) {
						return 0uL;
					}

					// a constituent of another type converts to the element / field type when both are scalars, else it does not fold
					for (u32 k = 0uL; k < out.size(); ++k) {
						const auto expected = t.kind == TypeKind::eArray ? t.element : t.fields[k].type;
						const auto actual = mod.exprs[e.operands[k].index].type;
						// equal spir-v type ids: structurally equal types (the writer deduplicates them)
						if (actual == expected || type(actual) == type(expected)) {
							continue;
						}

						const bool scalars = expected.index < mod.types.size() && actual.index < mod.types.size() &&
							mod.types[expected.index].kind == TypeKind::eScalar && mod.types[actual.index].kind == TypeKind::eScalar;
						const auto value = scalars ? scalarOf(e.operands[k], depth_ + 1uL) : None;
						if (not value.has_value()) {
							reason = "an element does not match the element type";
							return 0uL;
						}

						const auto converted = convert(mod.types[expected.index], *value);
						if (not converted.has_value()) {
							return 0uL;
						}

						out[k] = literal(expected, LiteralValue { *converted }, mod.exprs[e.operands[k].index].loc);
					}
					break;
				}
				default: return 0uL;
			}
			const auto id = w.constant(spv::Op::OpConstantComposite, type(e.type), out);
			if (t.kind == TypeKind::eVector || t.kind == TypeKind::eMatrix) {
				parts.insert_or_assign(id, std::move(out));
			}
			return id;
		};

		for (u32 i = 0uL; i < mod.constants.size(); ++i) {
			if (not reachedConstants[i]) {
				continue;
			}

			const auto& constant = mod.constants[i];
			if (constant.type.index >= mod.types.size()) {
				fail(codes::asl("internal-spirv-emit"), constant.loc, "constant '" + constant.name + "' has no type");
				continue;
			}

			const auto& desc = mod.types[constant.type.index];
			const auto at = constant.value.index < mod.exprs.size() ? mod.exprs[constant.value.index].loc : constant.loc;
			if (not constant.specId.has_value()) {
				// no name: the value is deduplicated, another constant may share it
				reason.clear();
				if (desc.kind == TypeKind::eScalar) {
					const auto value = scalarOf(constant.value, 0uL);
					constants[i] = value.has_value() ? literal(constant.type, LiteralValue { *value }, at) : 0uL;
				} else {
					constants[i] = fold(constant.value, 0uL);
				}
				if (constants[i] == 0uL) {
					fail(codes::asl("not-constant"), constant.loc, "initializer of constant '" + constant.name + "' does not fold to a constant value" + (reason.empty() ? String {} : ": " + reason));
				}
				continue;
			}

			// OpSpecConstant takes a literal default, so a constant-expression default folds here
			const auto value = scalarOf(constant.value, 0uL);
			if (not value.has_value()) {
				fail(
					codes::asl("not-constant"),
					constant.loc,
					"default of specialization constant '" + constant.name + "' does not fold to a literal"
				);
				continue;
			}

			const auto spec = std::ranges::find(layout.specConstants, *constant.specId, &SpecConstant::id);
			if (spec == layout.specConstants.end()) {
				fail(codes::asl("internal-inconsistent-layout"), constant.loc, "spec constant '" + constant.name + "' has no id");
				continue;
			}

			const auto typeId = type(constant.type);
			const LiteralValue folded { *value };
			if (desc.kind == TypeKind::eScalar && desc.scalar == ScalarKind::eBool) {
				const bool truth = std::visit([](auto value_) { return static_cast<bool>(value_); }, folded.value);
				constants[i] = w.constant(truth ? spv::Op::OpSpecConstantTrue : spv::Op::OpSpecConstantFalse, typeId, Ops {}, true);
			} else {
				Ops words {};
				if (not literalWords(constant.type, folded, words, at)) {
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
			// user names only: specialisations (which carry the generated params) stay unnamed
			if (fn.origin.empty()) {
				for (u32 p = 0uL; p < fn.params.size(); ++p) {
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
				mode(spv::ExecutionMode::OutputVertices, Ops { attributes.outputVertices });
				break;
			}
			case EntryKind::eTessEval: {
				model = spv::ExecutionModel::TessellationEvaluation;
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
