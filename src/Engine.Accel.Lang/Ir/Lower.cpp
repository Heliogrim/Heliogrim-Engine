#include "Lower.hpp"

#include <Engine.Common/Make.hpp>

#include "Interface.hpp"
#include "../Emitter/GlslEmitter.hpp"
#include "../Rep/Annotation.hpp"
#include "../Rep/Symbol.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr Type objectType { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown };

	[[nodiscard]] Type scalarType(ScalarType scalar_) {
		return Type { .category = TypeCategory::eScalar, .scalarType = scalar_ };
	}

	[[nodiscard]] Type legacyScalar(cref<ir::TypeDesc> desc_) {
		using enum ScalarType;
		switch (desc_.scalar) {
			// legacy types have no bool; a GLSL bool is 32 bits wide
			case ir::ScalarKind::eBool: return scalarType(eU32);
			case ir::ScalarKind::eUInt: {
				return desc_.bits == 8uL ? scalarType(eU8) : desc_.bits == 16uL ? scalarType(eU16) :
					desc_.bits == 32uL ? scalarType(eU32) : desc_.bits == 64uL ? scalarType(eU64) : objectType;
			}
			case ir::ScalarKind::eInt: {
				return desc_.bits == 8uL ? scalarType(eS8) : desc_.bits == 16uL ? scalarType(eS16) :
					desc_.bits == 32uL ? scalarType(eS32) : desc_.bits == 64uL ? scalarType(eS64) : objectType;
			}
			case ir::ScalarKind::eFloat: {
				return desc_.bits == 16uL ? scalarType(eF16) : desc_.bits == 32uL ? scalarType(eF32) :
					desc_.bits == 64uL ? scalarType(eF64) : objectType;
			}
			default: {
				return objectType;
			}
		}
	}

	/* shortcut: the legacy vector types are u8vec2-4 and f32vec3-4 only, anything else maps to an unknown object;
	 * extend `ScalarType` when a vertex input needs another format */
	[[nodiscard]] Type legacyType(cref<Vector<ir::TypeDesc>> types_, ir::TypeRef ref_) {
		if (ref_.index >= types_.size()) {
			return objectType;
		}
		const auto& desc = types_[ref_.index];
		switch (desc.kind) {
			case ir::TypeKind::eScalar: {
				return legacyScalar(desc);
			}
			case ir::TypeKind::eVector: {
				if (desc.scalar == ir::ScalarKind::eUInt && desc.bits == 8uL) {
					return desc.n == 2uL ? scalarType(ScalarType::eU8Vec2) : desc.n == 3uL ? scalarType(ScalarType::eU8Vec3) :
						desc.n == 4uL ? scalarType(ScalarType::eU8Vec4) : objectType;
				}
				if (desc.scalar == ir::ScalarKind::eFloat && desc.bits == 32uL) {
					return desc.n == 3uL ? scalarType(ScalarType::eF32Vec3) : desc.n == 4uL ? scalarType(ScalarType::eF32Vec4) : objectType;
				}
				return objectType;
			}
			case ir::TypeKind::eTexture: {
				using enum TextureType;
				const auto texture = desc.dim == ir::TextureDim::e1D ? eTexture1d : desc.dim == ir::TextureDim::e3D ? eTexture3d :
					desc.dim == ir::TextureDim::eCube ? eTextureCube : eTexture2d;
				return Type { .category = TypeCategory::eTexture, .textureType = texture };
			}
			case ir::TypeKind::eSampler: {
				return Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
			}
			case ir::TypeKind::eMatrix:
			case ir::TypeKind::eArray:
			case ir::TypeKind::eStruct:
			default: {
				return objectType;
			}
		}
	}

	template <AnnotationType Type_>
	[[nodiscard]] uptr<Annotation> wrap(mref<uptr<Annotation>> next_) {
		return make_uptr<SimpleAnnotation<Type_>>(std::move(next_));
	}
}

std::expected<Intermediate, ir::Diagnostics> ir::lowerToIntermediate(
	cref<Module> resolved_,
	cref<Layout> layout_,
	Dialect dialect_,
	cref<DenseMap<String, u32>> vertexOffsets_
) {
	if (dialect_ != Dialect::eVulkanGlsl460) {
		return std::unexpected(Diagnostics { Diagnostic { .message = "Only Vulkan GLSL 460 can be emitted." } });
	}

	Intermediate im {};
	auto diags = GlslEmitter {}.emit(resolved_, layout_, im.lang);
	im.lang.dialect = dialect_;

	u32 forwardInputs = 0uL;
	for (const auto& entry : computeInterface(resolved_)) {
		auto var = make_uptr<Variable>();
		var->type = legacyType(resolved_.types, entry.type);
		var->countTag = 1uL;

		uptr<Annotation> chain {};
		if (entry.mode == BindMode::eForward) {
			chain = make_uptr<SymbolIdAnnotation>(String { entry.id }, wrap<AnnotationType::eForwardLinkage>(nullptr));
			const auto index = entry.input ? forwardInputs++ : ~0uL;
			if (entry.input && resolved_.entry == EntryKind::eVertex) {
				const auto offset = vertexOffsets_.find(entry.id);
				if (offset == vertexOffsets_.end()) {
					diags.push_back(Diagnostic { .message = "Vertex input '" + entry.id + "' has no vertex offset." });
				} else if (index < layout_.inputLocations.size()) {
					// set -1: vertex buffer binding 0
					chain = make_uptr<VkBindLocationAnnotation>(
						-1L,
						static_cast<s32>(layout_.inputLocations[index]),
						static_cast<s32>(offset->second),
						std::move(chain)
					);
				}
			}
		} else {
			chain = wrap<AnnotationType::eExternalLinkage>(nullptr);
			if (entry.cls == BindClass::ePushConstant) {
				chain = wrap<AnnotationType::eReadonly>(wrap<AnnotationType::eConstant>(std::move(chain)));
			} else if (entry.cls == BindClass::eStorageBuffer) {
				chain = wrap<AnnotationType::eStorage>(std::move(chain));
			} else {
				chain = wrap<AnnotationType::eUniform>(std::move(chain));
			}
			chain = make_uptr<SymbolIdAnnotation>(String { entry.id }, std::move(chain));
		}
		var->annotation = std::move(chain);

		auto symbol = make_uptr<Symbol>(SymbolId::from(entry.id), VariableSymbol { SymbolType::eVariableSymbol, var.get() });
		symbol->symbolName = entry.id;
		(entry.input ? im.rep.globalScope.inbound : im.rep.globalScope.outbound).push_back(std::move(var));
		im.rep.symbolTable.insert(std::move(symbol));
	}

	if (not diags.empty()) {
		return std::unexpected(std::move(diags));
	}
	im.ir = make_smr<const Module>(new Module { resolved_ });
	return im;
}
