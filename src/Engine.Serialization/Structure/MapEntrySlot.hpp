#pragma once

#include "StructureSlot.hpp"
#include "SubstitutionSlot.hpp"

namespace hg::engine::serialization {
	template <typename KeyType_, typename ValueType_, template <typename, typename...> typename PackType_ = std::pair>
	class MapEntrySlot final :
		public TypedStructureSlotBase<PackType_<KeyType_, ValueType_>> {
	public:
		using this_type = MapEntrySlot<KeyType_, ValueType_, PackType_>;
		using underlying_type = TypedStructureSlotBase<PackType_<KeyType_, ValueType_>>;

		using value_type = typename underlying_type::value_type;

		using entry_key_type = KeyType_;
		using entry_key_slot_type = typename SubstitutionSlot<entry_key_type>::type;
		using entry_value_type = ValueType_;
		using entry_value_slot_type = typename SubstitutionSlot<entry_value_type>::type;

	public:
		explicit MapEntrySlot(mref<StructureSlotState> state_) noexcept :
			underlying_type(std::move(state_)) {
			this_type::_state.header = StructureSlotHeader::from<StructureSlotType::eMapEntry>();
		}

		~MapEntrySlot() override {
			if (this_type::_state.flags & StructureSlotStateFlag::eDirty) {
				this_type::leave();
			}
		}

	public:
		[[nodiscard]] StructureSlotType getSlotType() const noexcept override {
			return StructureSlotType::eMapEntry;
		}

		[[nodiscard]] bool validateType() const noexcept override {
			return this_type::_state.header.type == StructureSlotType::eMapEntry;
		}

	public:
		void operator<<(cref<value_type> value_) override {

			auto archive = this_type::_state.root->archive;

			constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
			s64 offset = this_type::_state.offset + off;

			/**/

			StructureSlotState keyState {
				StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
				offset,
				StructureSlotHeader::from<StructureSlotType::eUndefined>(),
				this_type::shared_from_this(),
				this_type::_state.root
			};

			entry_key_slot_type keySlot { std::move(keyState) };

			/**/

			keySlot.writeHeader();
			keySlot.enter();

			keySlot << value_.first;

			keySlot.leave();
			keySlot.writeHeader();

			/**/

			offset += keySlot.getSlotHeader().size;
			offset += sizeof(StructureSlotHeader::type);

			if (not keySlot.getSlotHeader().sizeInferred) {
				offset += sizeof(StructureSlotHeader::size);
			}

			/**/

			StructureSlotState valueState {
				StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
				offset,
				StructureSlotHeader::from<StructureSlotType::eUndefined>(),
				this_type::shared_from_this(),
				this_type::_state.root
			};

			entry_value_slot_type valueSlot { std::move(valueState) };

			/**/

			valueSlot.writeHeader();
			valueSlot.enter();

			valueSlot << value_.second;

			valueSlot.leave();
			valueSlot.writeHeader();

			/**/

			offset += valueSlot.getSlotHeader().size;
			offset += sizeof(StructureSlotHeader::type);

			if (not valueSlot.getSlotHeader().sizeInferred) {
				offset += sizeof(StructureSlotHeader::size);
			}

			const auto start = this_type::_state.offset + off;
			this_type::_state.header.size = offset - start;
		}

		SlotOpResult operator>>(ref<value_type> value_) override {

			if (this_type::_state.header.size <= 0) {
				return SlotOpResult::eFailed;
			}

			/**/

			constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
			s64 offset = this_type::_state.offset + off;

			/**/

			StructureSlotState keyState {
				StructureSlotStateFlag::eImmutable,
				offset,
				StructureSlotHeader::from<StructureSlotType::eUndefined>(),
				this_type::shared_from_this(),
				this_type::_state.root
			};

			entry_key_slot_type keySlot { std::move(keyState) };

			/**/

			keySlot.readHeader();
			keySlot.enter();

			keySlot >> value_.first;

			keySlot.leave();

			/**/

			offset += keySlot.getSlotHeader().size;
			offset += sizeof(StructureSlotHeader::type);

			if (not keySlot.getSlotHeader().sizeInferred) {
				offset += sizeof(StructureSlotHeader::size);
			}

			/**/

			StructureSlotState valueState {
				StructureSlotStateFlag::eImmutable,
				offset,
				StructureSlotHeader::from<StructureSlotType::eUndefined>(),
				this_type::shared_from_this(),
				this_type::_state.root
			};

			entry_value_slot_type valueSlot { std::move(valueState) };

			/**/

			valueSlot.readHeader();
			valueSlot.enter();

			valueSlot >> value_.second;

			valueSlot.leave();

			/**/

			return SlotOpResult::eSuccess;
		}
	};
}
