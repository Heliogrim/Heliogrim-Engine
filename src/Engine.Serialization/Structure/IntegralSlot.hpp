#pragma once

#include <Engine.Resource.Archive/Archive.hpp>

#include "StructureSlot.hpp"

namespace hg::engine::serialization {
	template <std::integral Type_>
	struct IntegralSlotTraits;

	template <std::integral IntegralType_>
	class IntegralSlot final :
		public TypedStructureSlotBase<IntegralType_> {
	public:
		using this_type = IntegralSlot<IntegralType_>;
		using underlying_type = TypedStructureSlotBase<IntegralType_>;

		using typename underlying_type::value_type;
		using trait_type = IntegralSlotTraits<IntegralType_>;

	public:
		explicit IntegralSlot(cref<StructureSlotState> state_) noexcept :
			underlying_type(state_) {
			this_type::_state.header = StructureSlotHeader::from<trait_type::slot_type>();
		}

		explicit IntegralSlot(mref<StructureSlotState> state_) noexcept :
			underlying_type(std::move(state_)) {
			this_type::_state.header = StructureSlotHeader::from<trait_type::slot_type>();
		}

	public:
		[[nodiscard]] StructureSlotType getSlotType() const noexcept override {
			return trait_type::slot_type;
		}

		[[nodiscard]] bool validateType() const noexcept override {
			return this_type::_state.header.type == trait_type::slot_type;
		}

	public:
		void enter() override {
			auto archive = this_type::_state.root->archive;

			constexpr s64 off = sizeof(trait_type::slot_type);
			archive->seek(this_type::_state.offset + off);
		}

	public:
		void operator<<(cref<value_type> value_) override {
			auto archive = this_type::_state.root->archive;
			(*archive) << value_;
		}

		SlotOpResult operator>>(ref<value_type> value_) override {
			auto archive = this_type::_state.root->archive;
			(*archive) >> value_;
			return SlotOpResult::eSuccess;
		}
	};

	/**/

	template <>
	struct IntegralSlotTraits<bool> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eBool;
	};

	template <>
	struct IntegralSlotTraits<u8> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eU8;
	};

	template <>
	struct IntegralSlotTraits<u16> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eU16;
	};

	template <>
	struct IntegralSlotTraits<u32> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eU32;
	};

	template <>
	struct IntegralSlotTraits<u64> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eU64;
	};

	template <>
	struct IntegralSlotTraits<s8> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eS8;
	};

	template <>
	struct IntegralSlotTraits<s16> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eS16;
	};

	template <>
	struct IntegralSlotTraits<s32> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eS32;
	};

	template <>
	struct IntegralSlotTraits<s64> {
		constexpr static StructureSlotType slot_type = StructureSlotType::eS64;
	};
}
