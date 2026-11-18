#include <algorithm>
#include <bit>
#include <variant>

#include "SpirvEmitter.Impl.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	namespace {
		template <typename Type_>
		[[nodiscard]] Type_ numberOf(ref<const LiteralValue> literal_);

		struct Format {
			StringView name;
			spv::ImageFormat format;
			ScalarKind scalar;
		};

		constexpr Format formats[] {
			{ "rgba32f", spv::ImageFormat::Rgba32f, ScalarKind::eFloat },
			{ "rgba16f", spv::ImageFormat::Rgba16f, ScalarKind::eFloat },
			{ "rg32f", spv::ImageFormat::Rg32f, ScalarKind::eFloat },
			{ "rg16f", spv::ImageFormat::Rg16f, ScalarKind::eFloat },
			{ "r32f", spv::ImageFormat::R32f, ScalarKind::eFloat },
			{ "r16f", spv::ImageFormat::R16f, ScalarKind::eFloat },
			{ "rgba8", spv::ImageFormat::Rgba8, ScalarKind::eFloat },
			{ "rgba8_snorm", spv::ImageFormat::Rgba8Snorm, ScalarKind::eFloat },
			{ "rgba32ui", spv::ImageFormat::Rgba32ui, ScalarKind::eUInt },
			{ "rgba16ui", spv::ImageFormat::Rgba16ui, ScalarKind::eUInt },
			{ "rgba8ui", spv::ImageFormat::Rgba8ui, ScalarKind::eUInt },
			{ "r32ui", spv::ImageFormat::R32ui, ScalarKind::eUInt },
			{ "rgba32i", spv::ImageFormat::Rgba32i, ScalarKind::eInt },
			{ "rgba16i", spv::ImageFormat::Rgba16i, ScalarKind::eInt },
			{ "rgba8i", spv::ImageFormat::Rgba8i, ScalarKind::eInt },
			{ "r32i", spv::ImageFormat::R32i, ScalarKind::eInt }
		};
	}

	u32 Impl::type(const TypeRef type_) {
		if (type_.index >= mod.types.size()) {
			fail(codes::asl("internal-spirv-emit"), {}, "type index out of range");
			return 0uL;
		}

		if (valueTypes[type_.index] != 0uL) {
			return valueTypes[type_.index];
		}

		const auto& desc = mod.types[type_.index];
		u32 id = 0uL;
		switch (desc.kind) {
			case TypeKind::eScalar: {
				id = scalar(desc.scalar, desc.bits);
				break;
			}
			case TypeKind::eVector: {
				id = w.type(spv::Op::OpTypeVector, Ops { scalar(desc.scalar, desc.bits), desc.n });
				break;
			}
			case TypeKind::eMatrix: {
				const auto column = w.type(spv::Op::OpTypeVector, Ops { scalar(ScalarKind::eFloat, desc.bits), desc.m });
				id = w.type(spv::Op::OpTypeMatrix, Ops { column, desc.n });
				break;
			}
			case TypeKind::eArray: {
				const auto element = type(desc.element);
				id = desc.count > 0uL ?
					w.type(spv::Op::OpTypeArray, Ops { element, u32Constant(desc.count) }) :
					w.type(spv::Op::OpTypeRuntimeArray, Ops { element });
				break;
			}
			case TypeKind::eStruct: {
				Ops members {};
				for (const auto& field : desc.fields) {
					members.emplace_back(type(field.type));
				}
				// fresh: equally shaped structs keep their own names
				id = w.type(spv::Op::OpTypeStruct, members, true);
				names(id, desc);
				break;
			}
			case TypeKind::eTexture: {
				id = image(desc, ScalarKind::eFloat, 1uL, spv::ImageFormat::Unknown);
				break;
			}
			case TypeKind::eImage: {
				const auto* format = std::ranges::find(formats, StringView { desc.name }, &Format::name);
				if (format == std::ranges::end(formats)) {
					fail(codes::asl("internal-spirv-emit"), {}, "storage image format '" + desc.name + "' has no SPIR-V format");
					return 0uL;
				}

				using enum spv::ImageFormat;
				if (format->format == Rg32f || format->format == Rg16f || format->format == R16f) {
					require(spv::Capability::StorageImageExtendedFormats);
				}
				id = image(desc, format->scalar, 2uL, format->format);
				break;
			}
			case TypeKind::eSampler: {
				id = w.type(spv::Op::OpTypeSampler, Ops {});
				break;
			}
			case TypeKind::eRegion: {
				fail(codes::asl("internal-spirv-emit"), {}, "region type left after reference lowering");
				return 0uL;
			}
		}
		valueTypes[type_.index] = id;
		return id;
	}

	u32 Impl::laidOut(const TypeRef type_, const bool std140_) {
		if (type_.index >= mod.types.size()) {
			fail(codes::asl("internal-spirv-emit"), {}, "type index out of range");
			return 0uL;
		}

		const auto key = std::pair { type_.index, std140_ };
		if (const auto it = laidOutTypes.find(key); it != laidOutTypes.end()) {
			return it->second;
		}

		const auto& desc = mod.types[type_.index];
		u32 id = 0uL;
		switch (desc.kind) {
			case TypeKind::eScalar: {
				id = desc.scalar == ScalarKind::eBool ? u32Type() : type(type_);
				break;
			}
			case TypeKind::eVector: {
				id = desc.scalar == ScalarKind::eBool ? w.type(spv::Op::OpTypeVector, Ops { u32Type(), desc.n }) : type(type_);
				break;
			}
			case TypeKind::eMatrix: {
				id = type(type_);
				break;
			}
			case TypeKind::eArray: {
				const auto element = laidOut(desc.element, std140_);
				const auto stride = arrayStride(mod.types, type_, std140_);
				if (stride == 0uL) {
					fail(codes::asl("internal-spirv-emit"), {}, "array type has no explicit layout");
					return 0uL;
				}

				// fresh: the stride decoration makes it distinct from the value array
				id = desc.count > 0uL ?
					w.type(spv::Op::OpTypeArray, Ops { element, u32Constant(desc.count) }, true) :
					w.type(spv::Op::OpTypeRuntimeArray, Ops { element }, true);
				w.annotation(spv::Op::OpDecorate, Ops { id, w32(spv::Decoration::ArrayStride), stride });
				break;
			}
			case TypeKind::eStruct: {
				id = structure(type_, std140_, 0uL, false);
				break;
			}
			default: {
				fail(codes::asl("internal-spirv-emit"), {}, "opaque type inside a buffer block");
				return 0uL;
			}
		}
		laidOutTypes.emplace(key, id);
		return id;
	}

	u32 Impl::block(const TypeRef type_, const bool std140_, const u32 bias_) {
		if (type_.index < mod.types.size() && mod.types[type_.index].kind == TypeKind::eStruct) {
			return structure(type_, std140_, bias_, true);
		}

		const auto member = laidOut(type_, std140_);
		const auto id = w.type(spv::Op::OpTypeStruct, Ops { member }, true);
		memberLayout(id, 0uL, bias_, type_, std140_);
		w.annotation(spv::Op::OpDecorate, Ops { id, w32(spv::Decoration::Block) });
		return id;
	}

	u32 Impl::pointer(const spv::StorageClass storage_, const u32 pointee_) {
		return w.type(spv::Op::OpTypePointer, Ops { w32(storage_), pointee_ });
	}

	u32 Impl::scalar(const ScalarKind kind_, const u32 bits_) {
		if (kind_ == ScalarKind::eBool) {
			return boolType();
		}

		if (kind_ == ScalarKind::eFloat) {
			if (bits_ == 64uL) {
				require(spv::Capability::Float64);
			} else if (bits_ == 16uL) {
				require(spv::Capability::Float16);
			}
			return w.type(spv::Op::OpTypeFloat, Ops { bits_ });
		}

		if (bits_ == 64uL) {
			require(spv::Capability::Int64);
		} else if (bits_ == 16uL) {
			require(spv::Capability::Int16);
		} else if (bits_ == 8uL) {
			require(spv::Capability::Int8);
		}
		return w.type(spv::Op::OpTypeInt, Ops { bits_, kind_ == ScalarKind::eInt ? 1uL : 0uL });
	}

	u32 Impl::voidType() {
		return w.type(spv::Op::OpTypeVoid, Ops {});
	}

	u32 Impl::boolType() {
		return w.type(spv::Op::OpTypeBool, Ops {});
	}

	u32 Impl::u32Type() {
		return w.type(spv::Op::OpTypeInt, Ops { 32uL, 0uL });
	}

	u32 Impl::u32Constant(const u32 value_) {
		return w.constant(spv::Op::OpConstant, u32Type(), Ops { value_ });
	}

	u32 Impl::literal(const TypeRef type_, ref<const LiteralValue> literal_, const SourceLoc loc_) {
		if (type_.index >= mod.types.size()) {
			fail(codes::asl("internal-spirv-emit"), loc_, "constant of a non-scalar type");
			return 0uL;
		}

		return literal(mod.types[type_.index], literal_, loc_);
	}

	u32 Impl::literal(ref<const TypeDesc> scalar_, ref<const LiteralValue> literal_, const SourceLoc loc_) {
		if (scalar_.kind == TypeKind::eScalar && scalar_.scalar == ScalarKind::eBool) {
			return w.constant(numberOf<bool>(literal_) ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse, boolType(), Ops {});
		}

		Ops words {};
		if (not literalWords(scalar_, literal_, words, loc_)) {
			return 0uL;
		}

		return w.constant(spv::Op::OpConstant, scalar(scalar_.scalar, scalar_.bits), words);
	}

	bool Impl::literalWords(const TypeRef type_, ref<const LiteralValue> literal_, ref<Ops> out_, const SourceLoc loc_) {
		if (type_.index >= mod.types.size()) {
			fail(codes::asl("internal-spirv-emit"), loc_, "constant of a non-scalar type");
			return false;
		}

		return literalWords(mod.types[type_.index], literal_, out_, loc_);
	}

	bool Impl::literalWords(ref<const TypeDesc> desc, ref<const LiteralValue> literal_, ref<Ops> out_, const SourceLoc loc_) {
		if (desc.kind != TypeKind::eScalar) {
			fail(codes::asl("internal-spirv-emit"), loc_, "constant of a non-scalar type");
			return false;
		}

		switch (desc.scalar) {
			case ScalarKind::eBool: {
				out_.emplace_back(numberOf<bool>(literal_) ? 1uL : 0uL);
				return true;
			}
			case ScalarKind::eInt:
			case ScalarKind::eUInt: {
				if (const auto* const real = std::get_if<f64>(&literal_.value); real != nullptr && not fitsInteger(
					*real,
					desc.scalar,
					desc.bits
				)) {
					fail(codes::asl("not-constant"), loc_, "a float outside the integer range does not convert");
					return false;
				}

				// signed narrow values keep their sign extension in the low word, as SPIR-V requires
				auto bits = desc.scalar == ScalarKind::eInt ? static_cast<u64>(numberOf<s64>(literal_)) : numberOf<u64>(literal_);
				// two's complement at the declared width: signed sign-extends, unsigned masks
				if (desc.bits > 0uL && desc.bits < 64uL) {
					const auto shift = 64uL - desc.bits;
					bits = desc.scalar == ScalarKind::eInt ?
						static_cast<u64>(static_cast<s64>(bits << shift) >> shift) :
						bits & ((1uLL << desc.bits) - 1uLL);
				}
				out_.emplace_back(static_cast<u32>(bits));
				if (desc.bits > 32uL) {
					out_.emplace_back(static_cast<u32>(bits >> 32uLL));
				}
				return true;
			}
			case ScalarKind::eFloat: {
				const auto value = numberOf<f64>(literal_);
				if (desc.bits == 32uL) {
					out_.emplace_back(std::bit_cast<u32>(static_cast<float>(value)));
					return true;
				}

				if (desc.bits == 64uL) {
					const auto bits = std::bit_cast<u64>(static_cast<double>(value));
					out_.emplace_back(static_cast<u32>(bits));
					out_.emplace_back(static_cast<u32>(bits >> 32uLL));
					return true;
				}

				// f16: round the f64 value straight to nearest even half (no f32 step: that would round twice), low 16 bits of one word
				const auto f = std::bit_cast<u64>(static_cast<double>(value));
				const auto sign = static_cast<u32>(f >> 48uLL) & 0x8000u;
				const auto biased = static_cast<u32>(f >> 52uLL) & 0x7FFu;
				const auto exp = static_cast<s32>(biased) - 1008;
				auto mant = f & 0xFFFFFFFFFFFFFuLL;
				const auto round = [](const u64 h_, const u64 rem_, const u64 halfway_) {
					return static_cast<u32>(rem_ > halfway_ || (rem_ == halfway_ && (h_ & 1uLL) != 0uLL) ? h_ + 1uLL : h_);
				};
				u32 half {};
				if (biased == 0x7FFu) {
					half = 0x7C00u | (mant != 0uLL ? 0x200u : 0u);
				} else if (exp >= 31) {
					half = 0x7C00u;
				} else if (exp <= 0) {
					if (exp >= -10) {
						mant |= 1uLL << 52uLL;
						const auto shift = static_cast<u64>(43 - exp);
						half = round(mant >> shift, mant & ((1uLL << shift) - 1uLL), 1uLL << (shift - 1uLL));
					}
				} else {
					half = round((static_cast<u64>(exp) << 10uLL) | (mant >> 42uLL), mant & ((1uLL << 42uLL) - 1uLL), 1uLL << 41uLL);
				}
				out_.emplace_back(sign | half);
				return true;
			}
		}
		return false;
	}

	void Impl::require(const spv::Capability capability_) {
		capabilities.emplace(capability_);
	}

	void Impl::fail(ref<const DiagnosticCodeInfo> code_, SourceLoc loc_, String message_) {
		// no own loc: point at the entry function
		if (loc_.offset == 0uL && loc_.length == 0uL && loc_.line == 1uL && loc_.column == 1uL && entry < mod.functions.size()) {
			loc_ = mod.functions[entry].loc;
		}
		diagnostics.emplace_back(Diagnostic::error(code_, "module '" + mod.name + "': " + std::move(message_)).at(spanIn(mod, loc_)));
	}

	void Impl::nameOf(const u32 id_, const StringView name_) {
		if (tier < Tier::eDev || name_.empty()) {
			return;
		}

		Ops operands { id_ };
		SpirvWriter::string(operands, name_);
		w.name(spv::Op::OpName, operands);
	}

	u32 Impl::image(ref<const TypeDesc> desc_, const ScalarKind sampled_, const u32 sampledMode_, const spv::ImageFormat format_) {
		auto dim = spv::Dim::Dim2D;
		u32 arrayed = 0uL;
		switch (desc_.dim) {
			case TextureDim::e1D: {
				dim = spv::Dim::Dim1D;
				require(sampledMode_ == 1uL ? spv::Capability::Sampled1D : spv::Capability::Image1D);
				break;
			}
			case TextureDim::e2D: {
				break;
			}
			case TextureDim::e3D: {
				dim = spv::Dim::Dim3D;
				break;
			}
			case TextureDim::eCube: {
				dim = spv::Dim::Cube;
				break;
			}
			case TextureDim::e2DArray: {
				arrayed = 1uL;
				break;
			}
		}
		return w.type(
			spv::Op::OpTypeImage,
			Ops { scalar(sampled_, 32uL), w32(dim), 0uL, arrayed, 0uL, sampledMode_, w32(format_) }
		);
	}

	u32 Impl::structure(const TypeRef type_, const bool std140_, const u32 bias_, const bool block_) {
		const auto& desc = mod.types[type_.index];
		Ops members {};
		for (const auto& field : desc.fields) {
			members.emplace_back(laidOut(field.type, std140_));
		}
		const auto id = w.type(spv::Op::OpTypeStruct, members, true);
		// bias (push range offset): the first member sits at it, later ones at their absolute std430 alignment, as GLSL
		const auto offsets = std140_ ? std140Offsets(mod.types, type_) : std430Offsets(mod.types, type_, bias_);
		if (offsets.size() != desc.fields.size()) {
			fail(codes::asl("internal-spirv-emit"), {}, "struct '" + desc.name + "' has no explicit layout");
			return id;
		}

		for (u32 i = 0uL; i < offsets.size(); ++i) {
			memberLayout(id, i, offsets[i], desc.fields[i].type, std140_);
		}
		if (block_) {
			w.annotation(spv::Op::OpDecorate, Ops { id, w32(spv::Decoration::Block) });
		}
		names(id, desc);
		return id;
	}

	void Impl::memberLayout(const u32 struct_, const u32 member_, const u32 offset_, const TypeRef type_, const bool std140_) {
		w.annotation(spv::Op::OpMemberDecorate, Ops { struct_, member_, w32(spv::Decoration::Offset), offset_ });
		auto inner = type_;
		while (inner.index < mod.types.size() && mod.types[inner.index].kind == TypeKind::eArray) {
			inner = mod.types[inner.index].element;
		}
		if (inner.index < mod.types.size() && mod.types[inner.index].kind == TypeKind::eMatrix) {
			w.annotation(spv::Op::OpMemberDecorate, Ops { struct_, member_, w32(spv::Decoration::ColMajor) });
			w.annotation(
				spv::Op::OpMemberDecorate,
				Ops { struct_, member_, w32(spv::Decoration::MatrixStride), matrixStride(mod.types, inner, std140_) }
			);
		}
	}

	void Impl::names(const u32 id_, ref<const TypeDesc> desc_) {
		if (tier < Tier::eDev) {
			return;
		}

		nameOf(id_, desc_.name);
		for (u32 i = 0uL; i < desc_.fields.size(); ++i) {
			if (desc_.fields[i].name.empty()) {
				continue;
			}

			Ops operands { id_, i };
			SpirvWriter::string(operands, desc_.fields[i].name);
			w.name(spv::Op::OpMemberName, operands);
		}
	}

	/**/

	namespace {
		template <typename Type_>
		[[nodiscard]] Type_ numberOf(ref<const LiteralValue> literal_) {
			return std::visit([](auto value_) { return static_cast<Type_>(value_); }, literal_.value);
		}
	}
}
