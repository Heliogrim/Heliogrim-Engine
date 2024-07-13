#pragma once

#include <Engine.Common/Meta/Concept.hpp>

#include "StructureSlot.hpp"
#include "MapEntrySlot.hpp"

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
		MapSlot(mref<StructureSlotState> state_) :
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

			if (this_type::_state.header.size <= 0) {
				return SlotOpResult::eFailed;
			}

			/**/

			auto archive = this_type::_state.root->archive;

			/**/

			u64 storedCount = ~0uLL;
			(*archive) >> storedCount;

			assert(storedCount != ~0uLL);

			/**/

			auto end = std::ranges::end(value_);
			std::insert_iterator<std::remove_reference_t<decltype(value_)>> inserter { value_, end };

			constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
			constexpr s64 ioff = sizeof(storedCount);

			s64 offset = this_type::_state.offset + off + ioff;

			for (u64 i = 0; i < storedCount; ++i) {

				StructureSlotState state {
					StructureSlotStateFlag::eImmutable,
					offset,
					StructureSlotHeader::from<StructureSlotType::eMapEntry>(),
					this_type::shared_from_this(),
					this_type::_state.root
				};

				auto slot = make_sptr<entry_type>(std::move(state));

				/**/

				slot->readHeader();
				slot->enter();

				typename entry_type::value_type dummy {};
				if (((*slot) >> dummy) == SlotOpResult::eSuccess) {
					inserter = std::move(dummy);
				}

				slot->leave();

				/**/

				offset += slot->getSlotHeader().size;
				offset += sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
			}

			return SlotOpResult::eSuccess;
		}
	};
}
