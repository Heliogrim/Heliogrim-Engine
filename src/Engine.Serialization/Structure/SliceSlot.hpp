#pragma once

#include "StructureSlot.hpp"
#include "SubstitutionSlot.hpp"

namespace hg {
	template <class Type_>
	concept HasResizeFnc = requires(Type_ obj_, u64 count_) {
		{ obj_.resize(count_) };
	};
}

namespace hg::engine::serialization {
	template <typename ValueType_, template <typename...> typename SliceType_>
	class SliceSlot final :
		public TypedStructureSlotBase<SliceType_<ValueType_>> {
	public:
		using this_type = SliceSlot<ValueType_, SliceType_>;
		using underlying_type = TypedStructureSlotBase<SliceType_<ValueType_>>;

		using value_type = typename underlying_type::value_type;

		using entry_type = typename SubstitutionSlot<ValueType_>::type;
		using slice_type = value_type;
		using slice_value_type = std::decay_t<ValueType_>;

	public:
		explicit SliceSlot(mref<StructureSlotState> state_) noexcept :
			underlying_type(std::move(state_)) {
			this_type::_state.header = StructureSlotHeader::from<StructureSlotType::eSlice>();
		}

		~SliceSlot() override {
			if (this_type::_state.flags & StructureSlotStateFlag::eDirty) {
				this_type::leave();
			}
		}

	public:
		[[nodiscard]] StructureSlotType getSlotType() const noexcept override {
			return StructureSlotType::eSlice;
		}

		[[nodiscard]] bool validateType() const noexcept override {
			return this_type::_state.header.type == StructureSlotType::eSlice;
		}

	public:
		void operator<<(cref<value_type> value_) override {

			auto archive = this_type::_state.root->archive;

			/* Write dummy / placeholder to archive and safe position */
			const auto countMarker = archive->tell();
			u64 count = 0uLL;
			(*archive) << count;

			/**/

			auto begin = std::ranges::begin(value_);
			const auto end = std::ranges::end(value_);

			constexpr s64 off = sizeof(/* StructureSlotHeader::type */StructureSlotType) + sizeof(
				/* StructureSlotHeader::size */s64);
			constexpr s64 ioff = sizeof(count);

			s64 offset = this_type::_state.offset + off + ioff;

			for (auto it = begin; it != end; ++it, ++count) {

				StructureSlotState state {
					StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
					offset,
					StructureSlotHeader::from<StructureSlotType::eUndefined>(),
					this_type::shared_from_this(),
					this_type::_state.root
				};

				entry_type slot { std::move(state) };

				/**/

				slot.writeHeader();
				slot.enter();

				slot << *it;

				slot.leave();
				slot.writeHeader();

				/**/

				offset += slot.getSlotHeader().size;
				offset += sizeof(/* StructureSlotHeader::type */StructureSlotType);

				if (not slot.getSlotHeader().sizeInferred) {
					offset += sizeof(/* StructureSlotHeader::size */s64);
				}
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
			if (header.type != StructureSlotType::eSlice) {
				this_type::fail(
					ReadError::Kind::eTypeMismatch,
					this_type::_state.offset,
					this_type::slotSpan(header),
					"slot is not a slice"
				);
				return SlotOpResult::eFailed;
			}

			if (header.size < static_cast<s64>(sizeof(u64))) {
				this_type::fail(
					ReadError::Kind::eMalformed,
					this_type::_state.offset,
					this_type::slotSpan(header),
					"slice too small for its count"
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
				this_type::fail(ReadError::Kind::eMalformed, this_type::payloadBegin(), sizeof(u64), "slice count exceeds its bytes");
				return SlotOpResult::eFailed;
			}

			/**/

			value_type tmp {};

			if constexpr (HasResizeFnc<value_type>) {
				if (not this_type::charge(storedCount * sizeof(slice_value_type))) {
					return SlotOpResult::eFailed;
				}
				tmp.resize(storedCount);

			} else {
				if (storedCount != static_cast<u64>(std::ranges::size(tmp))) {
					this_type::fail(
						ReadError::Kind::eMalformed,
						this_type::payloadBegin(),
						sizeof(u64),
						"slice count differs from the fixed extent"
					);
					return SlotOpResult::eFailed;
				}
			}

			s64 offset = this_type::payloadBegin() + static_cast<s64>(sizeof(u64));

			auto it = std::ranges::begin(tmp);
			for (u64 i = 0; i < storedCount; ++i, ++it) {

				StructureSlotState state {
					StructureSlotStateFlag::eImmutable,
					offset,
					StructureSlotHeader::from<StructureSlotType::eUndefined>(),
					this_type::shared_from_this(),
					this_type::_state.root
				};

				entry_type slot { std::move(state) };

				if (not slot.readHeader()) {
					return SlotOpResult::eFailed;
				}

				slot.enter();
				const auto result = slot >> (*it);
				slot.leave();

				if (result != SlotOpResult::eSuccess) {
					return SlotOpResult::eFailed;
				}

				offset = slot.payloadEnd();
			}

			/**/

			value_ = std::move(tmp);
			return SlotOpResult::eSuccess;
		}
	};
}
