#pragma once


#include "MigrateError.hpp"
#include "../Load/LoadContext.hpp"
#include "../Structure/StructScopedSlot.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class BufferArchive;
}

namespace hg::engine::serialization {
	using MigrateFn = Result<void, MigrateError> (*)(ref<const StructScopedSlot> old_, ref<StructScopedSlot> new_);

	struct Migration {
		type_id type;
		/* Upgrades `from` to `from + 1`. */
		u16 from;
		/* nullptr = native step, `hydrate` branches on `slot.revision()`. */
		MigrateFn fn;
	};

	/* The record after the bytes steps; owns the scratch archive its slot reads from. */
	class Migrated {
	public:
		explicit Migrated(mref<StructScopedSlot> slot_);

		Migrated(mref<uptr<resource::BufferArchive>> scratch_, mref<StructScopedSlot> slot_);

		Migrated(mref<Migrated>) noexcept = default;

		~Migrated();

	private:
		/* Heap-held so the slot's archive pointer survives moves of `Migrated`. */
		uptr<resource::BufferArchive> _scratch;
		StructScopedSlot _slot;

	public:
		[[nodiscard]] ref<const StructScopedSlot> slot() const noexcept;
	};

	class MigrationRegistry {
	private:
		DenseMap<type_id, Vector<MigrateFn>> _chains;

	public:
		/* Boot time: asserts steps per type are contiguous from 0 and native steps come last. */
		void add(std::span<const Migration> steps_);

		[[nodiscard]] u16 revision(const type_id type_) const;

		/* nullopt on failure or skip; the reason is already in the context. */
		[[nodiscard]] Opt<Migrated> migrateToCurrent(
			const type_id type_,
			ref<const StructScopedSlot> slot_,
			ref<LoadContext> context_
		) const;

		/* Writes `@rev` only when the type's revision is above 0. */
		void writeRevision(ref<StructScopedSlot> slot_, const type_id type_) const;
	};

	/* Copies every entry of `old_` byte for byte, except `@rev` and the listed keys. */
	void forwardEntries(
		ref<const StructScopedSlot> old_,
		ref<StructScopedSlot> new_,
		std::initializer_list<StringView> except_
	);
}
