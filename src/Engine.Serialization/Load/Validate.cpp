#include "Validate.hpp"


#include "../Structure/SlotCursor.hpp"

import std;
import Engine.Resource.Archive;

using namespace hg::engine::serialization;
using namespace hg;

namespace {
	struct Walker {
		ref<engine::resource::Archive> archive;
		Opt<ReadError> error = None;

		bool fail(const ReadError::Kind kind_, const s64 offset_, const s64 length_, const ptr<const char> detail_) {
			error = SomeVal<ReadError>(kind_, offset_, length_, String { detail_ });
			return false;
		}

		bool header(const SlotCursor cursor_, const s64 at_, ref<StructureSlotHeader> out_) {
			error = readSlotHeader(archive, cursor_, at_, out_);
			return not error.has_value();
		}

		bool count(const s64 at_, ref<const StructureSlotHeader> header_, ref<u64> out_) {
			if (header_.size < static_cast<s64>(sizeof(u64))) {
				return fail(ReadError::Kind::eMalformed, at_, header_.size, "container too small for its count");
			}

			archive.seek(at_);
			archive >> out_;
			if (archive.hasError()) {
				return fail(ReadError::Kind::eTruncated, at_, sizeof(u64), "container count past the end of the archive");
			}

			if (out_ > static_cast<u64>(header_.size) - sizeof(u64)) {
				return fail(ReadError::Kind::eMalformed, at_, sizeof(u64), "container count exceeds its bytes");
			}

			return true;
		}

		bool children(const SlotCursor cursor_, s64 at_, const u64 count_, const u32 depth_) {
			for (u64 i = 0; i < count_; ++i) {
				if (not slot(cursor_, at_, depth_, at_)) {
					return false;
				}
			}

			return true;
		}

		bool slot(const SlotCursor cursor_, const s64 at_, const u32 depth_, ref<s64> next_) {

			StructureSlotHeader header {};
			if (not this->header(cursor_, at_, header)) {
				return false;
			}

			const s64 begin = at_ + static_cast<s64>(sizeof(StructureSlotType)) + (
				header.sizeInferred ? 0LL : static_cast<s64>(sizeof(s64)));
			const s64 end = begin + (header.size > 0 ? header.size : 0LL);
			const SlotCursor inner { begin, end };
			next_ = end;

			switch (header.type) {
				case StructureSlotType::eStruct:
				case StructureSlotType::eSeq:
				case StructureSlotType::eSlice:
				case StructureSlotType::eMap: {
					if (depth_ >= LoadContext::maxDepth) {
						return fail(ReadError::Kind::eDepth, at_, end - at_, "nesting deeper than the limit");
					}
					break;
				}
				default: {
					break;
				}
			}

			switch (header.type) {
				case StructureSlotType::eStruct: {
					std::unordered_set<String> keys {};
					s64 off = begin;
					while (off < end) {
						StructureSlotHeader key {};
						if (not this->header(inner, off, key)) {
							return false;
						}

						if (key.type != StructureSlotType::eString) {
							return fail(ReadError::Kind::eMalformed, off, end - off, "struct key is not a string");
						}

						String name(static_cast<size_t>(key.size), '\0');
						archive.seek(off + 9);
						archive.serializeBytes(name.data(), name.size(), engine::resource::ArchiveStreamMode::eLoad);
						if (archive.hasError()) {
							return fail(ReadError::Kind::eTruncated, off, 9 + key.size, "struct key past the end of the archive");
						}

						if (not keys.emplace(std::move(name)).second) {
							return fail(ReadError::Kind::eDuplicateKey, off, 9 + key.size, "duplicate struct key");
						}

						off += 9 + key.size;
						if (not slot(inner, off, depth_ + 1, off)) {
							return false;
						}
					}

					return true;
				}
				case StructureSlotType::eSeq: {
					if (header.size == 0) {
						return true;
					}
					[[fallthrough]];
				}
				case StructureSlotType::eSlice:
				case StructureSlotType::eMap: {
					u64 n = 0;
					return count(begin, header, n) && children(inner, begin + 8, n, depth_ + 1);
				}
				case StructureSlotType::eMapEntry: {
					return children(inner, begin, 2uLL, depth_);
				}
				default: {
					return true;
				}
			}
		}
	};
}

Result<void, ReadError> hg::engine::serialization::validate(
	ref<resource::Archive> archive_,
	const s64 rootOffset_,
	ref<LoadContext> context_
) {
	const auto total = archive_.totalSize();
	const SlotCursor root { rootOffset_, total < 0 ? (std::numeric_limits<s64>::max)() : total };

	Walker walker { archive_ };
	s64 next = rootOffset_;
	if (walker.slot(root, rootOffset_, 0uL, next)) {
		return {};
	}

	context_.fail(*walker.error);
	return Unexpected<ReadError> { std::move(*walker.error) };
}
