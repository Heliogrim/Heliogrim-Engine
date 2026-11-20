#include "SlotCursor.hpp"

#include <Engine.Common/Optional.hpp>
#include <Engine.Resource.Archive/Archive.hpp>

using namespace hg::engine::serialization;
using namespace hg;

bool SlotCursor::fits(const s64 at_, const s64 len_) const noexcept {
	return len_ >= 0 && at_ >= begin && at_ <= end && len_ <= end - at_;
}

Opt<ReadError> hg::engine::serialization::readSlotHeader(
	ref<resource::Archive> archive_,
	const SlotCursor cursor_,
	const s64 at_,
	ref<StructureSlotHeader> out_
) {
	out_ = StructureSlotHeader {};

	constexpr s64 typeLen = sizeof(StructureSlotType);
	constexpr s64 sizeLen = sizeof(s64);

	if (not cursor_.fits(at_, typeLen)) {
		return SomeVal<ReadError>(ReadError::Kind::eTruncated, at_, typeLen, String { "slot header crosses its parent" });
	}

	archive_.seek(at_);
	auto type = StructureSlotType::eUndefined;
	archive_ >> type;
	if (archive_.hasError()) {
		return SomeVal<ReadError>(ReadError::Kind::eTruncated, at_, typeLen, String { "slot tag past the end of the archive" });
	}

	if (type == StructureSlotType::eUndefined) {
		return SomeVal<ReadError>(ReadError::Kind::eMalformed, at_, typeLen, String { "invalid slot tag" });
	}

	// Spec rule "unknown tag: size follows, skipped": any other tag reads an s64 size, bounded by the cursor below.
	auto header = StructureSlotHeader::from(type);
	if (header.sizeInferred) {
		if (not cursor_.fits(at_ + typeLen, header.size)) {
			return SomeVal<ReadError>(
				ReadError::Kind::eTruncated,
				at_,
				typeLen + header.size,
				String { "fixed-size slot crosses its parent" }
			);
		}

		out_ = header;
		return None;
	}

	if (not cursor_.fits(at_ + typeLen, sizeLen)) {
		return SomeVal<ReadError>(ReadError::Kind::eTruncated, at_, typeLen + sizeLen, String { "slot size crosses its parent" });
	}

	s64 size = -1;
	archive_ >> size;
	if (archive_.hasError()) {
		return SomeVal<ReadError>(ReadError::Kind::eTruncated, at_, typeLen + sizeLen, String { "slot size past the end of the archive" });
	}

	/* eRecord is only valid as the empty placeholder an unfilled insertRecord leaves behind. */
	if (type == StructureSlotType::eRecord && size != 0) {
		return SomeVal<ReadError>(ReadError::Kind::eMalformed, at_, typeLen + sizeLen, String { "non-empty record placeholder" });
	}

	if (size < 0 || not cursor_.fits(at_ + typeLen + sizeLen, size)) {
		return SomeVal<ReadError>(
			ReadError::Kind::eMalformed,
			at_,
			typeLen + sizeLen,
			String { "slot size exceeds the bytes left in its parent" }
		);
	}

	header.size = size;
	out_ = header;
	return None;
}
