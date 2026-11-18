#include <algorithm>

#include "SpirvEmitter.Impl.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	namespace {
		/* narrowest scalar width inside a type (32 for opaque types) */
		[[nodiscard]] u32 narrowest(ref<const Module> mod_, TypeRef type_) {
			if (type_.index >= mod_.types.size()) {
				return 32uL;
			}

			const auto& d = mod_.types[type_.index];
			switch (d.kind) {
				case TypeKind::eScalar:
				case TypeKind::eVector:
				case TypeKind::eMatrix: return d.kind == TypeKind::eScalar && d.scalar == ScalarKind::eBool ? 32uL : d.bits;
				case TypeKind::eArray:
				case TypeKind::eRegion: return narrowest(mod_, d.element);
				case TypeKind::eStruct: {
					u32 bits = 32uL;
					for (const auto& field : d.fields) {
						bits = std::min(bits, narrowest(mod_, field.type));
					}
					return bits;
				}
				default: return 32uL;
			}
		}
	}

	void Impl::features() {
		if (mod.entry == EntryKind::eTessCtrl || mod.entry == EntryKind::eTessEval) {
			require(spv::Capability::Tessellation);
		}
		if (mod.entry == EntryKind::eGeometry) {
			require(spv::Capability::Geometry);
		}
		if (builtinVars.contains("frag_depth") && entry < functions.size()) {
			w.executionMode(spv::Op::OpExecutionMode, Ops { functions[entry].id, w32(spv::ExecutionMode::DepthReplacing) });
		}

		// arithmetic capabilities (Int8/16/64, Float16/64) come from `scalar`; storage of narrow types needs its own
		for (u32 i = 0uL; i < globals.size() && i < mod.bindings.size(); ++i) {
			const auto& global = globals[i];
			if (global.id == 0uL) {
				continue;
			}

			const auto bits = narrowest(mod, mod.bindings[i].type);
			if (bits >= 32uL) {
				continue;
			}

			const auto byte = bits == 8uL;
			switch (global.storage) {
				case spv::StorageClass::StorageBuffer: {
					require(byte ? spv::Capability::StorageBuffer8BitAccess : spv::Capability::StorageBuffer16BitAccess);
					break;
				}
				case spv::StorageClass::Uniform: {
					require(
						byte ? spv::Capability::UniformAndStorageBuffer8BitAccess : spv::Capability::UniformAndStorageBuffer16BitAccess
					);
					break;
				}
				case spv::StorageClass::PushConstant: {
					require(byte ? spv::Capability::StoragePushConstant8 : spv::Capability::StoragePushConstant16);
					break;
				}
				case spv::StorageClass::Input:
				case spv::StorageClass::Output: {
					if (byte) {
						fail(
							codes::asl("unsupported-feature"),
							mod.bindings[i].loc,
							"binding '" + mod.bindings[i].id + "' passes an 8-bit type through the stage interface"
						);
					} else {
						require(spv::Capability::StorageInputOutput16);
					}
					break;
				}
				default: break;
			}
		}
	}
}
