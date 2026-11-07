#include "GlslEmitter.hpp"

#include <algorithm>
#include <numeric>
#include <string>
#include <tuple>
#include <Engine.Common/Optional.hpp>

#include "../Ir/Builtin.hpp"

using namespace hg::engine::accel;
using namespace hg;

namespace {
	using namespace ::hg::engine::accel::lang;

	/* the parser nests at most 256 deep; this only stops hand-built cyclic or runaway arenas */
	constexpr u32 maxDepth = 1024uL;

	[[nodiscard]] ir::TypeDesc scalarDesc(ir::ScalarKind scalar_, u32 bits_) {
		ir::TypeDesc desc {};
		desc.kind = ir::TypeKind::eScalar;
		desc.scalar = scalar_;
		desc.bits = bits_;
		return desc;
	}

	[[nodiscard]] ir::TypeDesc vectorDesc(ir::ScalarKind scalar_, u32 bits_, u32 n_) {
		auto desc = scalarDesc(scalar_, bits_);
		desc.kind = n_ == 1uL ? ir::TypeKind::eScalar : ir::TypeKind::eVector;
		desc.n = n_;
		return desc;
	}


	struct Emitted {
		String text;
		Opt<ir::TypeDesc> type;
	};

	constexpr u32 bit(GlslFeature feature_) {
		return static_cast<u32>(feature_);
	}

	constexpr u32 storage8 = bit(GlslFeature::eStorageBuffer8BitAccess) | bit(GlslFeature::eUniformAndStorageBuffer8BitAccess) |
		bit(GlslFeature::eStoragePushConstant8);
	constexpr u32 storage16 = bit(GlslFeature::eStorageBuffer16BitAccess) | bit(GlslFeature::eUniformAndStorageBuffer16BitAccess) |
		bit(GlslFeature::eStoragePushConstant16) | bit(GlslFeature::eStorageInputOutput16);

	/**
	 * The minimal extension set for the used features, sorted. Names are from the Khronos GLSL extension registry:
	 * GL_EXT_shader_explicit_arithmetic_types declares one sub-extension per width (`_int8`, `_int16`, `_int64`,
	 * `_float16`); `double` and `dvec`/`dmat` are core since GLSL 4.00 and only need the device feature
	 * (shaderFloat64). GL_EXT_shader_8bit_storage / GL_EXT_shader_16bit_storage allow the 8/16-bit types as members
	 * of buffer, uniform and push constant blocks (they do not declare the types: the arithmetic extension does).
	 */
	[[nodiscard]] Vector<String> extensionsOf(u32 features_) {
		using enum GlslFeature;
		Vector<String> names {};
		if ((features_ & bit(eShaderInt8)) != 0uL) {
			names.emplace_back("GL_EXT_shader_explicit_arithmetic_types_int8");
		}
		if ((features_ & bit(eShaderInt16)) != 0uL) {
			names.emplace_back("GL_EXT_shader_explicit_arithmetic_types_int16");
		}
		if ((features_ & bit(eShaderInt64)) != 0uL) {
			names.emplace_back("GL_EXT_shader_explicit_arithmetic_types_int64");
		}
		if ((features_ & bit(eShaderFloat16)) != 0uL) {
			names.emplace_back("GL_EXT_shader_explicit_arithmetic_types_float16");
		}
		if ((features_ & storage8) != 0uL) {
			names.emplace_back("GL_EXT_shader_8bit_storage");
		}
		if ((features_ & storage16) != 0uL) {
			names.emplace_back("GL_EXT_shader_16bit_storage");
		}
		std::ranges::sort(names);
		return names;
	}

	class Writer {
	public:
		Writer(ref<const ir::Module> module_, ref<const ir::Layout> layout_) :
			_module(module_),
			_layout(layout_),
			_structState(module_.types.size(), State::eNew) {}

		[[nodiscard]] ir::Diagnostics run(ref<String> out_, ref<GlslRequirements> requirements_) {
			bindings();
			constants();
			functions();

			requirements_ = GlslRequirements { .features = _features, .extensions = extensionsOf(_features) };
				if ((_builtinRequirements & ir::eRequireMultiview) != 0u) {
					requirements_.extensions.emplace_back("GL_EXT_multiview");
				}
				if ((_builtinRequirements & ir::eRequireSparse) != 0u) {
					requirements_.extensions.emplace_back("GL_ARB_sparse_texture2");
				}
				if ((_builtinRequirements & ir::eRequireSparseClamp) != 0u) {
					requirements_.extensions.emplace_back("GL_ARB_sparse_texture_clamp");
				}
				std::ranges::sort(requirements_.extensions);
			out_ = "#version 460\n";
			for (const auto& extension : requirements_.extensions) {
				out_ += "#extension " + extension + " : require\n";
			}
			out_ += "\nlayout(std430) buffer;\n";
				if (_module.entry == ir::EntryKind::eCompute) {
					const auto& size = _module.attributes.localSize;
					out_ += "layout(local_size_x = " + std::to_string(size[0]) + ", local_size_y = " + std::to_string(size[1]) +
						", local_size_z = " + std::to_string(size[2]) + ") in;\n";
				}
				if (_module.entry == ir::EntryKind::eGeometry) {
					const auto& a = _module.attributes;
					out_ += "layout(" + String(primitiveName(a.geometryInput)) + ") in;\n";
					out_ += "layout(" + String(primitiveName(a.geometryOutput)) + ", max_vertices = " + std::to_string(a.maxVertices) + ") out;\n";
					if (a.invocations > 1uL) {
						out_ += "layout(invocations = " + std::to_string(a.invocations) + ") in;\n";
					}
				}
				if (_module.entry == ir::EntryKind::eTessCtrl) {
						out_ += "layout(vertices = " + std::to_string(_module.attributes.outputVertices) + ") out;\n";
					}
					if (_module.entry == ir::EntryKind::eTessEval) {
						out_ += tessEvalLayout(_module.attributes);
					}
					if (_module.entry == ir::EntryKind::eFragment && _module.attributes.earlyFragmentTests) {
					out_ += "layout(early_fragment_tests) in;\n";
				}
				out_ += "\n" + _structs + _bindings + "\n" + _constants + _functions;
			return std::move(_diags);
		}

	private:
		enum class State : u8 {
			eNew,
			eOpen,
			eDone
		};

		struct Nest {
			ref<u32> depth;

			explicit Nest(ref<u32> depth_) :
				depth(++depth_) {}

			~Nest() {
				--depth;
			}
		};

		ref<const ir::Module> _module;
		ref<const ir::Layout> _layout;
		ir::Diagnostics _diags;

		String _structs;
		String _bindings;
		String _constants;
		String _functions;
		u32 _features = 0uL;
		u32 _builtinRequirements = 0uL;

		Vector<State> _structState;
		/* emitted struct name -> its member text, to emit a name once and catch conflicting definitions */
		Vector<std::pair<String, String>> _structNames;
		/* locals and parameters in scope, innermost last */
		struct Local {
			String name;
			Opt<ir::TypeDesc> type;
			/* a reference alias: the place text its uses are replaced by */
			Opt<String> alias = None;
		};

		Vector<Local> _locals;
		u32 _refs = 0uL;
		u32 _depth = 0uL;

		void error(String message_) {
			_diags.emplace_back(ir::Diagnostic { .message = std::move(message_) });
		}

		[[nodiscard]] const ir::TypeDesc* typeAt(ir::TypeRef ref_) const {
			return ref_.index < _module.types.size() ? &_module.types[ref_.index] : nullptr;
		}

		[[nodiscard]] Opt<ir::TypeDesc> typeOf(ir::TypeRef ref_) const {
			const auto* desc = typeAt(ref_);
			return desc != nullptr ? Opt<ir::TypeDesc> { *desc } : None;
		}

		/* Types */

		/* floats are 16, 32 or 64 bits wide, integers 8, 16, 32 or 64 */
		[[nodiscard]] static bool validWidth(ref<const ir::TypeDesc> desc_) {
			return desc_.bits == 16uL || desc_.bits == 32uL || desc_.bits == 64uL || (desc_.bits == 8uL && desc_.scalar != ir::ScalarKind::eFloat);
		}

		/* records the device features a named scalar, vector or matrix type needs */
		void note(ref<const ir::TypeDesc> desc_) {
			using enum GlslFeature;
			const auto bits = desc_.bits;
			if (desc_.scalar == ir::ScalarKind::eFloat) {
				_features |= bits == 16uL ? static_cast<u32>(eShaderFloat16) : bits == 64uL ? static_cast<u32>(eShaderFloat64) : 0uL;
				return;
			}
			_features |= bits == 8uL ? static_cast<u32>(eShaderInt8) : bits == 16uL ? static_cast<u32>(eShaderInt16) :
				bits == 64uL ? static_cast<u32>(eShaderInt64) : 0uL;
		}

		[[nodiscard]] Opt<String> scalarName(ref<const ir::TypeDesc> desc_) {
			using enum ir::ScalarKind;
			if (desc_.scalar == eBool) {
				return String { "bool" };
			}
			if (not validWidth(desc_)) {
				return None;
			}
			note(desc_);
			const auto bits = std::to_string(desc_.bits);
			if (desc_.scalar == eFloat) {
				return String { desc_.bits == 32uL ? "float" : desc_.bits == 64uL ? "double" : "float16_t" };
			}
			const String base = desc_.scalar == eInt ? "int" : "uint";
			return desc_.bits == 32uL ? base : base + bits + "_t";
		}

		[[nodiscard]] Opt<String> vectorPrefix(ref<const ir::TypeDesc> desc_) {
			using enum ir::ScalarKind;
			if (desc_.scalar == eBool) {
				return String { "b" };
			}
			if (not validWidth(desc_)) {
				return None;
			}
			note(desc_);
			if (desc_.scalar == eFloat) {
				return String { desc_.bits == 32uL ? "" : desc_.bits == 64uL ? "d" : "f16" };
			}
			const String base = desc_.scalar == eInt ? "i" : "u";
			return desc_.bits == 32uL ? base : base + std::to_string(desc_.bits);
		}

		/* GLSL type name; arrays in type syntax (`float[4]`); None if GLSL cannot name it */
		[[nodiscard]] Opt<String> name(ref<const ir::TypeDesc> desc_, u32 depth_ = 0uL) {
			using enum ir::TypeKind;
			const auto inRange = [](u32 v_) { return v_ >= 2uL && v_ <= 4uL; };
			switch (desc_.kind) {
				case eScalar: {
					return scalarName(desc_);
				}
				case eVector: {
					const auto prefix = vectorPrefix(desc_);
					return prefix.has_value() && inRange(desc_.n) ? Opt<String> { *prefix + "vec" + std::to_string(desc_.n) } : None;
				}
				case eMatrix: {
					// GLSL has float matrices only (no integer or bool matrix)
					const auto floating = desc_.scalar == ir::ScalarKind::eFloat && validWidth(desc_);
					if (not floating || not inRange(desc_.n) || not inRange(desc_.m)) {
						return None;
					}
					note(desc_);
					const String base = desc_.bits == 64uL ? "dmat" : desc_.bits == 16uL ? "f16mat" : "mat";
					return desc_.n == desc_.m ?
						base + std::to_string(desc_.n) :
						base + std::to_string(desc_.n) + "x" + std::to_string(desc_.m);
				}
				case eArray: {
					const auto* element = typeAt(desc_.element);
					const auto inner = element != nullptr && depth_ < maxDepth ? name(*element, depth_ + 1uL) : None;
					return inner.has_value() ?
						Opt<String> { *inner + "[" + (desc_.count == 0uL ? String {} : std::to_string(desc_.count)) + "]" } :
						None;
				}
				case eStruct: {
					return desc_.name.empty() ? None : Opt<String> { desc_.name };
				}
				case eTexture: {
					const auto prefix = desc_.scalar == ir::ScalarKind::eInt ? "i" : desc_.scalar == ir::ScalarKind::eUInt ? "u" : "";
					return String { prefix } + "texture" + dimName(desc_.dim);
				}
				case eSampler: {
					return String { "sampler" };
				}
				case eImage: {
					return String { desc_.name == "r32ui" ? "u" : "" } + "image" + dimName(desc_.dim);
				}
				default: {
					return None;
				}
			}
		}

		[[nodiscard]] static String tessEvalLayout(ref<const ir::EntryAttributes> a) {
			const auto* domain = a.tessDomain == ir::TessDomain::eQuads ? "quads" :
				(a.tessDomain == ir::TessDomain::eIsolines ? "isolines" : "triangles");
			const auto* spacing = a.tessSpacing == ir::TessSpacing::eFractionalEven ? "fractional_even_spacing" :
				(a.tessSpacing == ir::TessSpacing::eFractionalOdd ? "fractional_odd_spacing" : "equal_spacing");
			return "layout(" + String(domain) + ", " + spacing + ", " + (a.clockwise ? "cw" : "ccw") + ") in;\n";
		}

		[[nodiscard]] static StringView primitiveName(ir::Primitive primitive_) {
			using enum ir::Primitive;
			switch (primitive_) {
				case ePoints: return "points";
				case eLines: return "lines";
				case eLinesAdjacency: return "lines_adjacency";
				case eTrianglesAdjacency: return "triangles_adjacency";
				case eLineStrip: return "line_strip";
				case eTriangleStrip: return "triangle_strip";
				default: return "triangles";
			}
		}

		[[nodiscard]] static String dimName(ir::TextureDim dim_) {
			using enum ir::TextureDim;
			return dim_ == e1D ? "1D" : dim_ == e3D ? "3D" : dim_ == eCube ? "Cube" : dim_ == e2DArray ? "2DArray" : "2D";
		}

		/* `T name[2][3]`: array dimensions on the declarator, outermost first */
		[[nodiscard]] Opt<String> declare(ir::TypeRef ref_, StringView var_) {
			String dims {};
			const auto* desc = typeAt(ref_);
			for (auto guard = 0uL; desc != nullptr && desc->kind == ir::TypeKind::eArray && guard < maxDepth; ++guard) {
				dims += "[" + (desc->count == 0uL ? String {} : std::to_string(desc->count)) + "]";
				desc = typeAt(desc->element);
			}
			const auto base = desc != nullptr ? name(*desc) : None;
			return base.has_value() ? Opt<String> { *base + " " + String { var_ } + dims } : None;
		}

		/* declares `var_` or reports that its type has no GLSL form */
		[[nodiscard]] Opt<String> declareOrReport(ir::TypeRef ref_, StringView var_) {
			auto decl = declare(ref_, var_);
			if (not decl.has_value()) {
				error("The type of '" + String { var_ } + "' has no GLSL form.");
			}
			return decl;
		}

		/* emits the named structs a type needs (depth first, so members precede their users) */
		void use(ir::TypeRef ref_, u32 depth_ = 0uL) {
			if (ref_.index >= _structState.size() || depth_ > maxDepth) {
				return;
			}
			auto& state = _structState[ref_.index];
			if (state == State::eOpen) {
				error("A struct type contains itself.");
				return;
			}
			if (state == State::eDone) {
				return;
			}
			const auto& desc = _module.types[ref_.index];
			if (desc.kind == ir::TypeKind::eArray) {
				state = State::eOpen;
				use(desc.element, depth_ + 1uL);
				_structState[ref_.index] = State::eDone;
				return;
			}
			if (desc.kind != ir::TypeKind::eStruct) {
				state = State::eDone;
				return;
			}

			state = State::eOpen;
			for (const auto& field : desc.fields) {
				use(field.type, depth_ + 1uL);
			}
			_structState[ref_.index] = State::eDone;

			// anonymous: a block body, written with its binding
			if (desc.name.empty()) {
				return;
			}
			String members {};
			for (const auto& field : desc.fields) {
				if (const auto decl = declareOrReport(field.type, field.name); decl.has_value()) {
					members += "\t" + *decl + ";\n";
				}
			}
			const auto known = std::ranges::find(_structNames, desc.name, &std::pair<String, String>::first);
			if (known != _structNames.end()) {
				if (known->second != members) {
					error("Conflicting definitions of struct '" + desc.name + "'.");
				}
				return;
			}
			_structNames.emplace_back(desc.name, members);
			_structs += "struct " + desc.name + " {\n" + members + "};\n\n";
		}

		/**
		 * Records the storage features for the 8/16-bit components of a type placed in a binding (directly or in
		 * a reachable struct). A zero 8-bit mask means the placement cannot hold 8-bit types (stage interface).
		 */
		void storageUse(ir::TypeRef ref_, u32 mask8_, u32 mask16_, ref<const String> id_, u32 depth_ = 0uL) {
			const auto* desc = typeAt(ref_);
			if (desc == nullptr || depth_ > maxDepth || not _diags.empty()) {
				return;
			}
			switch (desc->kind) {
				case ir::TypeKind::eScalar:
				case ir::TypeKind::eVector:
				case ir::TypeKind::eMatrix: {
					if (desc->scalar == ir::ScalarKind::eBool) {
						return;
					}
					if (desc->bits == 8uL && mask8_ == 0uL) {
						error("Binding '" + id_ + "' forwards an 8-bit type; GLSL has no 8-bit stage interface variables.");
					}
					_features |= desc->bits == 8uL ? mask8_ : desc->bits == 16uL ? mask16_ : 0uL;
					return;
				}
				case ir::TypeKind::eArray: {
					storageUse(desc->element, mask8_, mask16_, id_, depth_ + 1uL);
					return;
				}
				case ir::TypeKind::eStruct: {
					for (const auto& field : desc->fields) {
						storageUse(field.type, mask8_, mask16_, id_, depth_ + 1uL);
					}
					return;
				}
				case ir::TypeKind::eTexture:
				case ir::TypeKind::eSampler:
				default: {
					return;
				}
			}
		}

		/* Bindings */

		void bindings() {
			const auto& decls = _module.bindings;
			// interface order (computeInterface): inputs first, then id; forward inputs index `inputLocations` in it
			Vector<u32> order(decls.size());
			std::iota(order.begin(), order.end(), 0uL);
			const auto key = [&](u32 i_) {
				const auto& b = decls[i_];
				return std::forward_as_tuple(not b.input, b.id, b.mode, b.bindClass, b.implicit, b.type.index);
			};
			std::ranges::sort(order, [&](u32 a_, u32 b_) { return key(a_) < key(b_); });

			u32 forwardInputs = 0uL;
			u32 outputLocation = 0uL;
			u32 pushBlocks = 0uL;
			for (auto i = 0uz; i < order.size(); ++i) {
				const auto& binding = decls[order[i]];
				if (i > 0uz && decls[order[i - 1uz]].id == binding.id && decls[order[i - 1uz]].input == binding.input) {
					error("Binding '" + binding.id + "' is declared twice.");
					continue;
				}
				use(binding.type);

				if (binding.bindClass == ir::BindClass::eWorkgroup) {
					if (const auto decl = declareOrReport(binding.type, binding.localName); decl.has_value()) {
						_bindings += "shared " + *decl + ";\n";
					}
					continue;
				}

				if (binding.mode == ir::BindMode::eForward) {
					if (binding.input) {
						forward(binding, forwardInputs++);
					} else if (not binding.implicit) {
						if (const auto decl = declareOrReport(binding.type, binding.localName); decl.has_value()) {
							_bindings += "layout(location = " + std::to_string(outputLocation) + ") " + (binding.patch ? "patch " : "") +
								"out " + *decl + (binding.arrayed ? "[]" : "") + ";\n";
							storageUse(binding.type, 0uL, bit(GlslFeature::eStorageInputOutput16), binding.id);
						}
						outputLocation += ir::locationCount(_module.types, binding.type);
					}
					continue;
				}

				if (binding.implicit) {
					continue;
				}
				pushBlocks += binding.bindClass == ir::BindClass::ePushConstant ? 1uL : 0uL;
				resource(binding);
			}

			if (forwardInputs != _layout.inputLocations.size()) {
				error("The layout has " + std::to_string(_layout.inputLocations.size()) + " input locations, the module " +
					std::to_string(forwardInputs) + " forward inputs.");
			}
			if (pushBlocks > 1uL) {
				error("A stage has at most one push constant block, the module declares " + std::to_string(pushBlocks) + ".");
			}
		}

		void forward(ref<const ir::BindingDecl> binding_, u32 index_) {
			if (binding_.implicit) {
				return;
			}
			if (index_ >= _layout.inputLocations.size()) {
				error("The layout has no location for input '" + binding_.id + "'.");
				return;
			}
			const auto decl = declareOrReport(binding_.type, binding_.localName);
			if (not decl.has_value()) {
				return;
			}
			storageUse(binding_.type, 0uL, bit(GlslFeature::eStorageInputOutput16), binding_.id);
			// integer fragment inputs cannot be interpolated
			const auto* desc = typeAt(binding_.type);
			const auto integral = desc != nullptr && (desc->kind == ir::TypeKind::eScalar || desc->kind == ir::TypeKind::eVector) &&
				desc->scalar != ir::ScalarKind::eFloat;
			const auto flat = _module.entry == ir::EntryKind::eFragment && integral;
			_bindings += "layout(location = " + std::to_string(_layout.inputLocations[index_]) + ") " + (flat ? "flat " : "") +
				(binding_.patch ? "patch " : "") + "in " + *decl + (binding_.arrayed ? "[]" : "") + ";\n";
		}

		void resource(ref<const ir::BindingDecl> binding_) {
			using enum ir::BindClass;
			const auto* desc = typeAt(binding_.type);
			const auto kind = desc != nullptr ? Opt<ir::TypeKind> { desc->kind } : None;
			const auto opaque = kind == ir::TypeKind::eTexture || kind == ir::TypeKind::eSampler || kind == ir::TypeKind::eImage;

			if (binding_.bindClass == eNone) {
				error("Binding '" + binding_.id + "' has no class.");
				return;
			}
			const auto fits = binding_.bindClass == eTexture ?
				kind == ir::TypeKind::eTexture :
				binding_.bindClass == eSampler ?
				kind == ir::TypeKind::eSampler :
				binding_.bindClass == eStorageImage ?
				kind == ir::TypeKind::eImage :
				kind.has_value() && not opaque;
			if (not fits) {
				error("The type of binding '" + binding_.id + "' does not fit its class.");
				return;
			}
			if (not opaque) {
				using enum GlslFeature;
				const auto push = binding_.bindClass == ePushConstant;
				const auto buffer = binding_.bindClass == eStorageBuffer;
				storageUse(
					binding_.type,
					bit(push ? eStoragePushConstant8 : buffer ? eStorageBuffer8BitAccess : eUniformAndStorageBuffer8BitAccess),
					bit(push ? eStoragePushConstant16 : buffer ? eStorageBuffer16BitAccess : eUniformAndStorageBuffer16BitAccess),
					binding_.id
				);
			}

			if (binding_.bindClass == ePushConstant) {
				const auto range = std::ranges::find(_layout.push, binding_.id, &ir::PushRange::id);
				if (range == _layout.push.end() || range->size == 0uL) {
					error("Push constant '" + binding_.id + "' cannot be laid out.");
					return;
				}
				block("layout(push_constant) uniform", binding_, *desc);
				return;
			}

			const auto slot = std::ranges::find(_layout.slots, binding_.id, &ir::SlotAssignment::id);
			if (slot == _layout.slots.end()) {
				error("The layout has no slot for binding '" + binding_.id + "'.");
				return;
			}
			const auto packing = binding_.packing == ir::Packing::eStd140 ? ", std140" : binding_.packing == ir::Packing::eStd430 ? ", std430" : "";
			const auto head = "layout(set = " + std::to_string(slot->set) + ", binding = " + std::to_string(slot->binding) + packing + ")";
			if (kind == ir::TypeKind::eImage) {
				const auto access = binding_.access == ir::Access::eRead ? "readonly " : binding_.access == ir::Access::eWrite ? "writeonly " : "";
				if (const auto decl = declareOrReport(binding_.type, binding_.localName); decl.has_value()) {
					_bindings += "layout(set = " + std::to_string(slot->set) + ", binding = " + std::to_string(slot->binding) + ", " +
						desc->name + ") uniform " + access + *decl + ";\n";
				}
				return;
			}
			if (opaque) {
				if (const auto decl = declareOrReport(binding_.type, binding_.localName); decl.has_value()) {
					_bindings += head + " uniform " + *decl + ";\n";
				}
				return;
			}
			const auto qualifier = binding_.access == ir::Access::eRead && binding_.input ? " readonly" : binding_.access == ir::Access::eWrite ? " writeonly" : "";
			block(head + (binding_.bindClass == eStorageBuffer ? String { qualifier } + " buffer" : String { " uniform" }), binding_, *desc);
		}

		/* module constants in declaration order (an initializer only reads earlier ones); spec constants carry their constant_id */
		void constants() {
			for (const auto& constant : _module.constants) {
				const auto decl = declareOrReport(constant.type, constant.name);
				if (not decl.has_value()) {
					continue;
				}
				String head {};
				if (constant.specId.has_value()) {
					const auto it = std::ranges::find(_layout.specConstants, *constant.specId, &ir::SpecConstant::id);
					if (it == _layout.specConstants.end()) {
						error("The layout has no constant id for spec constant '" + *constant.specId + "'.");
						continue;
					}
					head = "layout(constant_id = " + std::to_string(it->constantId) + ") ";
				}
				_constants += head + "const " + *decl + " = " + expr(constant.value).text + ";\n";
			}
			if (not _constants.empty()) {
				_constants += "\n";
			}
		}

		/* an anonymous struct becomes the block body (instance name = local name), any other type its only member */
		void block(ref<const String> head_, ref<const ir::BindingDecl> binding_, ref<const ir::TypeDesc> desc_) {
			const auto& local = binding_.localName;
			if (desc_.kind == ir::TypeKind::eStruct && desc_.name.empty()) {
				String members {};
				for (const auto& field : desc_.fields) {
					if (const auto decl = declareOrReport(field.type, field.name); decl.has_value()) {
						members += "\t" + *decl + ";\n";
					}
				}
				_bindings += head_ + " " + local + "_block {\n" + members + "} " + local + ";\n";
				return;
			}
			if (const auto decl = declareOrReport(binding_.type, local); decl.has_value()) {
				_bindings += head_ + " " + local + "_block {\n\t" + *decl + ";\n};\n";
			}
		}

		/* Functions */

		[[nodiscard]] Opt<String> signature(ref<const ir::FunctionDecl> fn_) {
			const auto* ret = typeAt(fn_.ret);
			const auto retName = ret != nullptr ? name(*ret) : Opt<String> { "void" };
			if (not retName.has_value()) {
				error("The return type of '" + fn_.name + "' has no GLSL form.");
				return None;
			}
			String params {};
			for (const auto& param : fn_.params) {
				const auto decl = declareOrReport(param.type, param.name);
				if (not decl.has_value()) {
					return None;
				}
				params += (params.empty() ? "" : ", ") + String { param.kind == ir::ParamKind::eRefMut ? "inout " : "const in " } + *decl;
			}
			return *retName + " " + fn_.name + "(" + params + ")";
		}

		void functions() {
			bool hasMain = false;
			String prototypes {};
			String definitions {};
			for (const auto& fn : _module.functions) {
				if (not fn.body.has_value()) {
					continue;
				}
				for (const auto& param : fn.params) {
					use(param.type);
				}
				use(fn.ret);
				if (fn.name == "main") {
					hasMain = true;
					if (not fn.params.empty() || fn.ret.index != ir::TypeRef {}.index) {
						error("'main' takes no parameters and returns nothing.");
					}
				}
				const auto sig = signature(fn);
				if (not sig.has_value()) {
					continue;
				}
				if (fn.name != "main") {
					prototypes += *sig + ";\n";
				}

				_locals.clear();
				_refs = 0uL;
				for (const auto& param : fn.params) {
					_locals.push_back({ param.name, typeOf(param.type) });
				}
				definitions += "\n" + *sig + "\n";
				arm(fn.body->root, 0uL, definitions);
			}
			if (not hasMain) {
				definitions += "\nvoid main() {\n}\n";
			}
			_functions = prototypes + definitions;
		}

		/* a statement as a braced scope */
		void arm(ir::StmtRef ref_, u32 indent_, ref<String> out_) {
			const String pad(indent_, '\t');
			if (ref_.index < _module.stmts.size() && _module.stmts[ref_.index].kind == ir::StmtKind::eBlock) {
				statement(ref_, indent_, out_);
				return;
			}
			const auto mark = _locals.size();
			out_ += pad + "{\n";
			statement(ref_, indent_ + 1uL, out_);
			out_ += pad + "}\n";
			_locals.resize(mark);
		}

		void statement(ir::StmtRef ref_, u32 indent_, ref<String> out_) {
			// absent (~0) or out of range: nothing
			if (ref_.index >= _module.stmts.size()) {
				return;
			}
			const Nest nest { _depth };
			if (_depth > maxDepth) {
				error("The code nests too deeply.");
				return;
			}

			const auto& stmt = _module.stmts[ref_.index];
			const String pad(indent_, '\t');
			switch (stmt.kind) {
				case ir::StmtKind::eBlock: {
					const auto mark = _locals.size();
					out_ += pad + "{\n";
					for (const auto child : stmt.children) {
						statement(child, indent_ + 1uL, out_);
					}
					out_ += pad + "}\n";
					_locals.resize(mark);
					break;
				}
				case ir::StmtKind::eDeclareLocal: {
					declareLocal(stmt, pad, out_);
					break;
				}
				case ir::StmtKind::eDeclareRef: {
					const auto target = place(stmt.lhs, pad, out_);
					_locals.push_back({ stmt.name, target.type, target.text });
					break;
				}
				case ir::StmtKind::eAssign: {
					const auto lhs = expr(stmt.lhs);
					out_ += pad + lhs.text + (stmt.name.empty() ? String { " = " } : " " + stmt.name + " ") + expr(stmt.rhs).text + ";\n";
					break;
				}
				case ir::StmtKind::eExpr: {
					out_ += pad + expr(stmt.lhs).text + ";\n";
					break;
				}
				case ir::StmtKind::eReturn: {
					out_ += pad + "return" + (stmt.lhs.index < _module.exprs.size() ? " " + expr(stmt.lhs).text : String {}) + ";\n";
					break;
				}
				case ir::StmtKind::eIf: {
					out_ += pad + "if (" + expr(stmt.lhs).text + ")\n";
					arm(stmt.children.empty() ? ir::StmtRef {} : stmt.children[0], indent_, out_);
					if (stmt.children.size() > 1uz) {
						out_ += pad + "else\n";
						arm(stmt.children[1], indent_, out_);
					}
					break;
				}
				case ir::StmtKind::eWhile: {
					out_ += pad + "while (" + expr(stmt.lhs).text + ")\n";
					arm(stmt.children.empty() ? ir::StmtRef {} : stmt.children[0], indent_, out_);
					break;
				}
				case ir::StmtKind::eFor: {
					const auto mark = _locals.size();
					const auto inline_ = [&](u32 at_) {
						String text {};
						if (at_ < stmt.children.size()) {
							const auto child = stmt.children[at_];
							// an omitted init/step is an empty block; render nothing
							const auto omitted = child.index < _module.stmts.size() && _module.stmts[child.index].kind == ir::StmtKind::eBlock
								&& _module.stmts[child.index].children.empty();
							if (not omitted) {
								statement(child, 0uL, text);
							}
						}
						while (not text.empty() && (text.back() == '\n' || text.back() == ';' || text.back() == '\t')) {
							text.pop_back();
						}
						return text;
					};
					const auto init = inline_(0uL);
					const auto step = inline_(2uL);
					const auto cond = stmt.lhs.index < _module.exprs.size() ? expr(stmt.lhs).text : String {};
					out_ += pad + "for (" + init + "; " + cond + "; " + step + ")\n";
					arm(stmt.children.size() > 1uz ? stmt.children[1] : ir::StmtRef {}, indent_, out_);
					_locals.resize(mark);
					break;
				}
				case ir::StmtKind::eBreak: {
					out_ += pad + "break;\n";
					break;
				}
				case ir::StmtKind::eContinue: {
					out_ += pad + "continue;\n";
					break;
				}
				case ir::StmtKind::eDiscard: {
					out_ += pad + "discard;\n";
					break;
				}
				case ir::StmtKind::eWhen:
				default: {
					error("An unresolved @when statement cannot be emitted.");
					break;
				}
			}
		}

		void declareLocal(ref<const ir::Stmt> stmt_, ref<const String> pad_, ref<String> out_) {
			const auto init = stmt_.lhs.index < _module.exprs.size() ? Opt<Emitted> { expr(stmt_.lhs) } : None;
			Opt<ir::TypeDesc> type = typeOf(stmt_.type);
			Opt<String> decl = None;
			if (type.has_value()) {
				use(stmt_.type);
				decl = declareOrReport(stmt_.type, stmt_.name);
			} else if (init.has_value() && init->type.has_value()) {
				type = init->type;
				const auto typeName = name(*type);
				decl = typeName.has_value() ? Opt<String> { *typeName + " " + stmt_.name } : None;
				if (not decl.has_value()) {
					error("The type of '" + stmt_.name + "' has no GLSL form.");
				}
			} else {
				error("Cannot infer the type of local '" + stmt_.name + "'.");
			}
			if (decl.has_value()) {
				out_ += pad_ + (stmt_.mutable_ ? "" : "const ") + *decl + (init.has_value() ? " = " + init->text : String {}) + ";\n";
			}
			_locals.push_back({ stmt_.name, std::move(type) });
		}

		/* Expressions: every unary and binary is parenthesized, so source precedence carries over */

		[[nodiscard]] const Local* findLocal(ref<const String> name_) const {
			for (auto it = _locals.rbegin(); it != _locals.rend(); ++it) {
				if (it->name == name_) {
					return &*it;
				}
			}
			return nullptr;
		}

		[[nodiscard]] Opt<ir::TypeDesc> localType(ref<const String> name_) const {
			const auto* local = findLocal(name_);
			return local != nullptr ? local->type : None;
		}

		/* a place for an alias: non-constant indices are evaluated once, at the alias point */
		[[nodiscard]] Emitted place(ir::ExprRef ref_, ref<const String> pad_, ref<String> out_) {
			if (ref_.index >= _module.exprs.size()) {
				return expr(ref_);
			}
			const auto& e = _module.exprs[ref_.index];
			if (e.kind == ir::ExprKind::eField && not e.operands.empty()) {
				return { place(e.operands[0], pad_, out_).text + "." + e.name, typeOf(e.type) };
			}
			if (e.kind == ir::ExprKind::eIndex && e.operands.size() == 2uz) {
				const auto base = place(e.operands[0], pad_, out_);
				auto index = expr(e.operands[1]).text;
				if (e.operands[1].index < _module.exprs.size() && _module.exprs[e.operands[1].index].kind != ir::ExprKind::eLiteral) {
					const auto hoisted = "_hg_ref" + std::to_string(_refs++);
					out_ += pad_ + "const uint " + hoisted + " = " + index + ";\n";
					index = hoisted;
				}
				return { base.text + "[" + index + "]", typeOf(e.type) };
			}
			return expr(ref_);
		}

		[[nodiscard]] Emitted expr(ir::ExprRef ref_) {
			if (ref_.index >= _module.exprs.size()) {
				error("An expression is missing.");
				return { "0", None };
			}
			const Nest nest { _depth };
			if (_depth > maxDepth) {
				error("The code nests too deeply.");
				return { "0", None };
			}

			const auto& e = _module.exprs[ref_.index];
			const auto operand = [&](size_t i_) -> Emitted {
				return i_ < e.operands.size() ? expr(e.operands[i_]) : expr(ir::ExprRef {});
			};
			const auto arguments = [&] {
				String args {};
				for (const auto arg : e.operands) {
					args += (args.empty() ? "" : ", ") + expr(arg).text;
				}
				return args;
			};

			switch (e.kind) {
				case ir::ExprKind::eLiteral: {
					if (e.name == "true" || e.name == "false") {
						return { e.name, scalarDesc(ir::ScalarKind::eBool, 32uL) };
					}
					// hex digits may contain 'e'/'E'; a `u` suffix makes an integer unsigned
					const auto hex = e.name.starts_with("0x") || e.name.starts_with("0X");
					const auto floating = not hex && e.name.find_first_of(".eE") != String::npos;
					const auto floatType = typeOf(e.type).has_value() && typeOf(e.type)->kind == ir::TypeKind::eScalar && typeOf(e.type)->scalar == ir::ScalarKind::eFloat;
					const auto unsignedInt = not floating && (e.name.ends_with('u') || e.name.ends_with('U'));
					return {
						floatType && not floating && not hex ? e.name + ".0" : e.name,
						scalarDesc(floating || floatType ? ir::ScalarKind::eFloat : unsignedInt ? ir::ScalarKind::eUInt : ir::ScalarKind::eInt, 32uL)
					};
				}
				case ir::ExprKind::eNameRef: {
					return nameRef(e);
				}
				case ir::ExprKind::eBuiltin: {
					if (const auto* variable = ir::findBuiltinVariable(e.name); variable != nullptr && e.operands.empty()) {
						_builtinRequirements |= variable->requirement;
						return { String { variable->glsl }, vectorDesc(variable->scalar, 32uL, variable->n) };
					}
					if (const auto* fn = ir::findBuiltinFunction(e.name); fn != nullptr && not fn->signature.empty()) {
						_builtinRequirements |= fn->requirement;
						if (fn->signature.find("(tex,smp") != StringView::npos) {
							return textureCall(e, *fn);
						}
						return { String { fn->glsl } + "(" + arguments() + ")", typeOf(e.type) };
					}
					error("Unknown builtin '" + e.name + "'.");
					return { e.name, None };
				}
				case ir::ExprKind::eCall: {
					const auto* fn = e.function.index < _module.functions.size() ? &_module.functions[e.function.index] : nullptr;
					return { e.name + "(" + arguments() + ")", typeOf(e.type) };
				}
				case ir::ExprKind::eField: {
					auto base = operand(0uz);
					return { base.text + "." + e.name, typeOf(e.type) };
				}
				case ir::ExprKind::eIndex: {
					auto base = operand(0uz);
					const auto index = operand(1uz);
					const auto& baseExpr = _module.exprs[e.operands.front().index];
					const auto* builtinVar = baseExpr.kind == ir::ExprKind::eBuiltin ? ir::findBuiltinVariable(baseExpr.name) : nullptr;
					const String suffix = builtinVar != nullptr && builtinVar->arrayed ? ".gl_Position" : "";
					return { base.text + "[" + index.text + "]" + suffix, typeOf(e.type) };
				}
				case ir::ExprKind::eUnary: {
					auto inner = operand(0uz);
					return { "(" + e.name + inner.text + ")", typeOf(e.type) };
				}
				case ir::ExprKind::eBinary: {
					auto lhs = operand(0uz);
					auto rhs = operand(1uz);
					return { "(" + lhs.text + " " + e.name + " " + rhs.text + ")", typeOf(e.type) };
				}
				case ir::ExprKind::eTernary: {
					auto c = operand(0uz);
					auto a = operand(1uz);
					auto b = operand(2uz);
					return { "(" + c.text + " ? " + a.text + " : " + b.text + ")", typeOf(e.type) };
				}
				case ir::ExprKind::eConstruct: {
					use(e.type);
					const auto type = typeOf(e.type);
					const auto typeName = type.has_value() ? name(*type) : None;
					if (not typeName.has_value()) {
						error("Cannot construct '" + e.name + "' in GLSL.");
					}
					return { typeName.value_or(e.name) + "(" + arguments() + ")", type };
				}
				default: {
					return { "0", None };
				}
			}
		}

		[[nodiscard]] Emitted nameRef(ref<const ir::Expr> e_) {
			if (e_.local) {
				if (const auto* local = findLocal(e_.name); local != nullptr && local->alias.has_value()) {
					return { *local->alias, local->type };
				}
				return { e_.name, typeOf(e_.type).has_value() ? typeOf(e_.type) : localType(e_.name) };
			}
			if (e_.constant.index < _module.constants.size()) {
				const auto& constant = _module.constants[e_.constant.index];
				return { constant.name, typeOf(constant.type) };
			}
			const auto& decls = _module.bindings;
			const auto* binding = e_.binding.index < decls.size() ? &decls[e_.binding.index] : nullptr;
			if (binding == nullptr) {
				const auto it = std::ranges::find(decls, e_.name, &ir::BindingDecl::localName);
				binding = it != decls.end() ? &*it : nullptr;
			}
			if (binding == nullptr) {
				return { e_.name, typeOf(e_.type).has_value() ? typeOf(e_.type) : localType(e_.name) };
			}
			if (binding->implicit) {
				error("Implicit binding '" + binding->id + "' has no GLSL variable.");
			}
			return { binding->localName, typeOf(binding->type) };
		}

		/* `fn(texture, sampler, ...)` -> `glsl(sampler2D(texture, sampler), ...)`; the leading texture + sampler pair is combined */
		[[nodiscard]] Emitted textureCall(ref<const ir::Expr> e_, ref<const ir::BuiltinFunction> fn_) {
			if (e_.operands.size() < 2uz) {
				error("'" + e_.name + "' takes a texture and a sampler.");
				return { "0", None };
			}
			const auto texture = expr(e_.operands[0]);
			const auto sampler = expr(e_.operands[1]);
			if (not texture.type.has_value() || texture.type->kind != ir::TypeKind::eTexture) {
				error("The first argument of '" + e_.name + "' must be a texture.");
				return { "0", None };
			}
			const auto& tex = *texture.type;
			const auto prefix = tex.scalar == ir::ScalarKind::eInt ? "i" : tex.scalar == ir::ScalarKind::eUInt ? "u" : "";
			String text = String { fn_.glsl } + "(" + prefix + "sampler" + dimName(tex.dim) + "(" + texture.text + ", " + sampler.text + ")";
			for (auto i = 2uz; i < e_.operands.size(); ++i) {
				text += ", " + expr(e_.operands[i]).text;
			}
			return { text + ")", typeOf(e_.type) };
		}
	};
}

void lang::GlslEmitter::operator()(ref<const Representation>, Dialect dialect_, ref<Language> target_) const {
	// Legacy path: shader text comes from the Intermediate; IR output goes through emit().
	target_.dialect = dialect_;
}

lang::GlslEmitResult lang::GlslEmitter::emit(ref<const ir::Module> module_, ref<const ir::Layout> layout_, ref<Language> target_) const {
	String text {};
	GlslEmitResult result {};
	result.diagnostics = Writer { module_, layout_ }.run(text, result.requirements);
	if (not result.diagnostics.empty()) {
		result.requirements = {};
		return result;
	}
	target_.dialect = Dialect::eVulkanGlsl460;
	target_.text.emplace_back(TextBlock { std::move(text) });
	return result;
}
