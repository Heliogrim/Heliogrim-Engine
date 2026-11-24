#include "Migration.hpp"

#include "../Archive/StructuredArchive.hpp"
#include "../Structure/IntegralScopedSlot.hpp"
#include "../Structure/RootScopedSlot.hpp"
#include "../Structure/SlotCursor.hpp"
#include "../Structure/StructSlot.hpp"
#include "../Symbols.hpp"

import Engine.Asserts;
import Engine.Common;
import Engine.Resource.Archive;

using namespace hg::engine::serialization;
using namespace hg;

namespace {
	[[nodiscard]] s64 slotLength(const non_owning_rptr<StructureSlotBase> slot_) noexcept;

	void report(const non_owning_rptr<StructureSlotBase> slot_, mref<ReadError> error_);
}

/**/

Migrated::Migrated(mref<StructScopedSlot> slot_) :
	_scratch(),
	_slot(slot_) {}

Migrated::Migrated(mref<uptr<resource::BufferArchive>> scratch_, mref<StructScopedSlot> slot_) :
	_scratch(std::move(scratch_)),
	_slot(slot_) {}

Migrated::~Migrated() {
	// A later scratch archive may reuse this address; its read sets must start empty.
	if (_scratch != nullptr) {
		if (const auto context = _slot.slot()->getState().root->context; context != nullptr) {
			context->forgetArchive(static_cast<ptr<const resource::Archive>>(_scratch.get()));
		}
	}
}

ref<const StructScopedSlot> Migrated::slot() const noexcept {
	return _slot;
}

void MigrationRegistry::add(std::span<const Migration> steps_) {

	/* Boot-time registration only: a broken chain is a programming error, never file data. */
	DenseMap<type_id, Vector<Opt<MigrateFn>>> merged {};
	for (const auto& step : steps_) {
		auto& chain = merged[step.type];
		if (chain.empty()) {
			if (const auto it = _chains.find(step.type); it != _chains.end()) {
				chain.assign(it->second.begin(), it->second.end());
			}
		}

		if (chain.size() <= step.from) {
			chain.resize(static_cast<size_t>(step.from) + 1uLL);
		}
		assertrt(not chain[step.from].has_value());
		chain[step.from] = step.fn;
	}

	for (auto& [type, chain] : merged) {
		Vector<MigrateFn> flat {};
		flat.reserve(chain.size());

		assertrt(chain.size() <= 0xFFFFuLL);

		bool native = false;
		for (const auto& fn : chain) {
			assertrt(fn.has_value());
			assertrt(not native || *fn == nullptr);
			native = native || *fn == nullptr;
			flat.emplace_back(*fn);
		}

		_chains[type] = std::move(flat);
	}
}

u16 MigrationRegistry::revision(const type_id type_) const {
	const auto it = _chains.find(type_);
	return it != _chains.end() ? static_cast<u16>(it->second.size()) : u16 { 0 };
}

Opt<Migrated> MigrationRegistry::migrateToCurrent(
	const type_id type_,
	ref<const StructScopedSlot> slot_,
	ref<LoadContext> context_
) const {

	const auto origin = slot_.slot();
	const auto& originRoot = *origin->getState().root;
	const bool scratchOrigin = originRoot.originOffset >= 0;
	const s64 offset = scratchOrigin ? originRoot.originOffset : origin->getState().offset;
	const s64 length = scratchOrigin ? originRoot.originLength : slotLength(origin);

	const u16 stored = slot_.revision();
	if (context_.failed()) {
		return None;
	}

	const u16 current = revision(type_);
	if (stored > current) {
		const auto detail = std::format("stored revision {} is newer than current revision {}", stored, current);
		if (origin->getState().parent == nullptr) {
			context_.fail(ReadError { ReadError::Kind::eRevisionNewerRoot, offset, length, String { detail } });
		} else {
			context_.warn(ReadError { ReadError::Kind::eRevisionNewer, offset, length, String { detail } });
		}
		return None;
	}

	if (stored == current) {
		return SomeVal<Migrated>(StructScopedSlot { slot_ });
	}

	const auto& chain = _chains.at(type_);

	uptr<resource::BufferArchive> scratch {};
	auto cursor = SomeVal<StructScopedSlot>(slot_);

	for (u16 rev = stored; rev < current && chain[rev] != nullptr; ++rev) {

		auto next = make_uptr<resource::BufferArchive>(); {
			StructuredArchive writer { *next };
			auto out = writer.insertRootSlot().intoStruct();
			out.insertSlot<u16>(Symbols::Revision) << static_cast<u16>(rev + 1u);

			const auto result = chain[rev](*cursor, out);
			if (not result.has_value()) {
				context_.fail(ReadError { ReadError::Kind::eMigrationFailed, offset, length, result.error().message });
				return None;
			}
		}

		if (context_.failed()) {
			return None;
		}

		if (next->hasError()) {
			context_.fail(
				ReadError {
					ReadError::Kind::eMigrationFailed, offset, length,
					String { std::format("migration step from revision {} failed to write", rev) }
				}
			);
			return None;
		}

		const auto scratchSize = static_cast<u64>(std::max<s64>(next->totalSize(), 0LL));
		if (not context_.charge(scratchSize)) {
			context_.fail(
				ReadError { ReadError::Kind::eBudget, offset, length, String { "migration scratch exceeds the allocation budget" } }
			);
			return None;
		}

		next->seek(0);
		cursor.emplace(RootScopedSlot { *next, ScopedSlotStateFlag::eClean, &context_ }.asStruct());
		cursor->slot()->getState().root->originOffset = offset;
		cursor->slot()->getState().root->originLength = length;
		// The superseded intermediate is freed here; drop its read sets before its address can be reused.
		if (scratch != nullptr) {
			context_.forgetArchive(static_cast<ptr<const resource::Archive>>(scratch.get()));
		}
		scratch = std::move(next);
	}

	return SomeVal<Migrated>(std::move(scratch), std::move(*cursor));
}

void MigrationRegistry::writeRevision(ref<StructScopedSlot> slot_, const type_id type_) const {
	if (const u16 rev = revision(type_); rev > 0) {
		slot_.insertSlot<u16>(Symbols::Revision) << rev;
	}
}

void engine::serialization::forwardEntries(
	ref<const StructScopedSlot> old_,
	ref<StructScopedSlot> new_,
	std::initializer_list<StringView> except_
) {

	const auto source = old_.slot();
	if (source == nullptr || not source->hasPayload()) {
		return;
	}

	auto& from = *source->getState().root->archive;
	auto& to = *new_.slot()->getState().root->archive;
	const SlotCursor cursor { source->payloadBegin(), source->payloadEnd() };

	std::array<u8, 4096uLL> buffer {};
	s64 offset = cursor.begin;

	while (offset < cursor.end) {

		StructureSlotHeader keyHeader {};
		if (auto error = readSlotHeader(from, cursor, offset, keyHeader)) {
			report(source, std::move(*error));
			return;
		}

		if (keyHeader.type != StructureSlotType::eString) {
			report(
				source,
				ReadError { ReadError::Kind::eMalformed, offset, StructureSlotBase::slotSpan(keyHeader), "struct key is not a string" }
			);
			return;
		}

		if (const auto context = source->getState().root->context; context != nullptr && not context->charge(
			static_cast<u64>(keyHeader.size)
		)) {
			report(
				source,
				ReadError {
					ReadError::Kind::eBudget, offset, StructureSlotBase::slotSpan(keyHeader), "struct key exceeds the allocation budget"
				}
			);
			return;
		}

		String key(static_cast<size_t>(keyHeader.size), '\0');
		from.seek(offset + StructureSlotBase::slotSpan(keyHeader) - keyHeader.size);
		from.serializeBytes(reinterpret_cast<ptr<u8>>(key.data()), key.size(), resource::ArchiveStreamMode::eLoad);
		if (from.hasError()) {
			report(
				source,
				ReadError { ReadError::Kind::eTruncated, offset, StructureSlotBase::slotSpan(keyHeader), "struct key unreadable" }
			);
			return;
		}

		const s64 valueAt = offset + StructureSlotBase::slotSpan(keyHeader);
		StructureSlotHeader valueHeader {};
		if (auto error = readSlotHeader(from, cursor, valueAt, valueHeader)) {
			report(source, std::move(*error));
			return;
		}

		const s64 next = valueAt + StructureSlotBase::slotSpan(valueHeader);
		offset = next;

		if (key == Symbols::Revision.str || std::ranges::find(except_, StringView { key }) != except_.end()) {
			continue;
		}

		/* Raw copy keeps the value byte-identical; the record header and parent size are fixed up afterwards. */
		auto record = new_.insertRecordSlot(key);
		const auto target = record.slot();
		const s64 writeAt = target->getState().offset;

		for (s64 copied = 0; copied < next - valueAt;) {
			const auto chunk = static_cast<size_t>(std::min<s64>(next - valueAt - copied, static_cast<s64>(buffer.size())));
			from.seek(valueAt + copied);
			from.serializeBytes(buffer.data(), chunk, resource::ArchiveStreamMode::eLoad);
			to.seek(writeAt + copied);
			to.serializeBytes(buffer.data(), chunk, resource::ArchiveStreamMode::eStore);
			copied += static_cast<s64>(chunk);
		}

		if (from.hasError()) {
			report(source, ReadError { ReadError::Kind::eTruncated, valueAt, next - valueAt, "struct value unreadable" });
			target->getState().flags = StructureSlotStateFlag::eImmutable;
			return;
		}

		target->getSlotHeader() = valueHeader;
		new_.slot()->feedback(target);
		target->getState().flags = StructureSlotStateFlag::eImmutable;
	}
}

/**/

namespace {
	s64 slotLength(const non_owning_rptr<StructureSlotBase> slot_) noexcept {
		return slot_->hasPayload() ? slot_->payloadEnd() - slot_->getState().offset : 0LL;
	}

	void report(const non_owning_rptr<StructureSlotBase> slot_, mref<ReadError> error_) {
		const auto& root = *slot_->getState().root;
		if (root.originOffset >= 0) {
			error_.offset = root.originOffset;
			error_.length = root.originLength;
		}
		if (root.context != nullptr) {
			root.context->fail(std::move(error_));
		}
	}
}
