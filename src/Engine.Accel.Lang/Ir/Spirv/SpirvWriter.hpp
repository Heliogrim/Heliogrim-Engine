#pragma once

#include <map>
#include <span>
#include <spirv/unified1/spirv.hpp11>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::accel::lang::ir::spirv {
	/**
	 * Knows only opcodes, operands, ids and module layout order; no ASL types.
	 *
	 * Every section function appends one instruction whose operands are given verbatim (result ids included), obtained from `id()`.
	 * The debug part of the layout is two sections: `debug` (OpString, OpSource...) precedes `name` (OpName, OpMemberName, OpModuleProcessed).
	 */
	class SpirvWriter {
	public:
		[[nodiscard]] u32 id() noexcept;

		void capability(const spv::Op op_, const std::span<const u32> operands_);
		void extension(const spv::Op op_, const std::span<const u32> operands_);
		void extInstImport(const spv::Op op_, const std::span<const u32> operands_);
		void memoryModel(const spv::Op op_, const std::span<const u32> operands_);
		void entryPoint(const spv::Op op_, const std::span<const u32> operands_);
		void executionMode(const spv::Op op_, const std::span<const u32> operands_);
		void debug(const spv::Op op_, const std::span<const u32> operands_);
		void name(const spv::Op op_, const std::span<const u32> operands_);
		void annotation(const spv::Op op_, const std::span<const u32> operands_);
		/* types, constants and global variables, appended as given and never deduplicated */
		void global(const spv::Op op_, const std::span<const u32> operands_);
		void function(const spv::Op op_, const std::span<const u32> operands_);

		/**
		 * Type declaration, `operands_` exclude the result id. Equal opcode and operands return the same id unless `fresh_`
		 * (needed where decorations make structurally equal types distinct, e.g. block structs).
		 */
		[[nodiscard]] u32 type(const spv::Op op_, const std::span<const u32> operands_, const bool fresh_ = false);

		/** Constant declaration, `operands_` exclude result type and result id; deduplicated like `type`. */
		[[nodiscard]] u32 constant(const spv::Op op_, const u32 resultType_, const std::span<const u32> operands_, const bool fresh_ = false);

		/** Appends `value_` as nul-terminated UTF-8 packed little-endian, zero padded to a word. */
		static void string(ref<Vector<u32>> out_, const StringView value_);

		[[nodiscard]] Vector<u32> finish() const;

		/* an instruction over the 16-bit word count was dropped; the module is unusable */
		[[nodiscard]] bool overlong() const noexcept;

	private:
		u32 _bound = 1uL;
		Vector<u32> _capability, _extension, _extInstImport, _memoryModel, _entryPoint, _executionMode, _debug, _name, _annotation,
			_global, _function;
		std::map<Vector<u32>, u32> _dedup;
		bool _overlong = false;

	private:
		void append(ref<Vector<u32>> into_, const spv::Op op_, const std::span<const u32> operands_);
	};
}
