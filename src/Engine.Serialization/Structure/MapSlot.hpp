#pragma once

#include <algorithm>
#include <ranges>
#include <type_traits>

#include "MapEntrySlot.hpp"
#include "StructureSlot.hpp"

namespace hg::engine::serialization {
	template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
	class MapSlot final :
		public TypedStructureSlotBase<MapType_<KeyType_, ValueType_>> {
	public:
		using this_type = MapSlot<KeyType_, ValueType_, MapType_>;
		using underlying_type = TypedStructureSlotBase<MapType_<KeyType_, ValueType_>>;

		using value_type = typename underlying_type::value_type;

		using entry_type = MapEntrySlot<KeyType_, ValueType_>;

	public:
		explicit MapSlot(mref<StructureSlotState> state_) noexcept :
			underlying_type(std::move(state_)) {
			this_type::_state.header = StructureSlotHeader::from<StructureSlotType::eMap>();
		}

		~MapSlot() override {
			if (this_type::_state.flags & StructureSlotStateFlag::eDirty) {
				this_type::leave();
			}
		}

	public:
		[[nodiscard]] StructureSlotType getSlotType() const noexcept override {
			return StructureSlotType::eMap;
		}

		[[nodiscard]] bool validateType() const noexcept override {
			return this_type::_state.header.type == StructureSlotType::eMap;
		}

	public:
		void operator<<(cref<value_type> value_) override {

			auto archive = this_type::_state.root->archive;

			/* Write placeholder to archive and safe position */
			const auto countMarker = archive->tell();
			u64 count = 0uLL;
			(*archive) << count;

			/**/

			auto begin = std::ranges::begin(value_);
			const auto end = std::ranges::end(value_);

			const s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
			const s64 ioff = sizeof(count);

			s64 offset = this_type::_state.offset + off + ioff;

			for (auto it = begin; it != end; ++it, ++count) {

				StructureSlotState state {
					StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
					offset,
					StructureSlotHeader::from<StructureSlotType::eMapEntry>(),
					this_type::shared_from_this(),
					this_type::_state.root
				};

				auto slot = make_sptr<entry_type>(std::move(state));

				/**/

				slot->writeHeader();
				slot->enter();

				using map_iterator_type = typename value_type::iterator;
				using map_iterator_value_type = typename map_iterator_type::value_type;
				if constexpr (std::is_convertible_v<map_iterator_value_type, typename entry_type::value_type>) {
					(*slot) << *it;

				} else {
					static_assert(
						std::is_convertible_v<map_iterator_value_type, typename entry_type::value_type>,
						"Failed to decay iterator value type to matching pair type for serializable map entry."
					);
				}

				slot->leave();
				slot->writeHeader();

				/**/

				offset += slot->getSlotHeader().size;
				offset += sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
			}

			/**/

			const auto start = this_type::_state.offset + off;
			this_type::_state.header.size = offset - start;

			/**/

			const auto cursor = archive->tell();
			archive->seek(countMarker);
			(*archive) << count;

			/**/

			archive->seek(cursor);
		}

		SlotOpResult operator>>(ref<value_type> value_) override {

			if (not this_type::hasPayload()) {
				return SlotOpResult::eFailed;
			}

			const auto& header = this_type::_state.header;
			if (header.type != StructureSlotType::eMap) {
				this_type::fail(ReadError::Kind::eTypeMismatch, this_type::_state.offset, this_type::slotSpan(header), "slot is not a map");
				return SlotOpResult::eFailed;
			}

			if (header.size < static_cast<s64>(sizeof(u64))) {
				this_type::fail(
					ReadError::Kind::eMalformed,
					this_type::_state.offset,
					this_type::slotSpan(header),
					"map too small for its count"
				);
				return SlotOpResult::eFailed;
			}

			/**/

			auto archive = this_type::_state.root->archive;
			archive->seek(this_type::payloadBegin());

			u64 storedCount = 0uLL;
			(*archive) >> storedCount;

			if (not this_type::checkArchive(this_type::_state.offset, this_type::slotSpan(header))) {
				return SlotOpResult::eFailed;
			}

			if (storedCount > static_cast<u64>(header.size) - sizeof(u64)) {
				this_type::fail(ReadError::Kind::eMalformed, this_type::payloadBegin(), sizeof(u64), "map count exceeds its bytes");
				return SlotOpResult::eFailed;
			}

			/**/

			value_type tmp {};
			std::insert_iterator<value_type> inserter { tmp, std::ranges::end(tmp) };

			s64 offset = this_type::payloadBegin() + static_cast<s64>(sizeof(u64));

			for (u64 i = 0; i < storedCount; ++i) {

				StructureSlotState state {
					StructureSlotStateFlag::eImmutable,
					offset,
					StructureSlotHeader::from<StructureSlotType::eMapEntry>(),
					this_type::shared_from_this(),
					this_type::_state.root
				};

				auto slot = make_sptr<entry_type>(std::move(state));

				if (not slot->readHeader()) {
					return SlotOpResult::eFailed;
				}

				slot->enter();
				typename entry_type::value_type entry {};
				const auto result = (*slot) >> entry;
				slot->leave();

				if (result != SlotOpResult::eSuccess) {
					return SlotOpResult::eFailed;
				}

				inserter = std::move(entry);
				offset = slot->payloadEnd();
			}

			value_ = std::move(tmp);
			return SlotOpResult::eSuccess;
		}
	};
}
