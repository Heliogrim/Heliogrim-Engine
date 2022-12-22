#pragma once

#include "StructureSlot.hpp"

namespace ember::engine::serialization {
    template <_STD integral Type_>
    struct IntegralSlotTraits;

    template <_STD integral IntegralType_>
    class IntegralSlot :
        public TypedStructureSlotBase<IntegralType_> {
    public:
        using this_type = IntegralSlot<IntegralType_>;
        using underlying_type = TypedStructureSlotBase<IntegralType_>;

        using typename underlying_type::value_type;
        using trait_type = IntegralSlotTraits<IntegralType_>;

    public:
        IntegralSlot(cref<StructureSlotState> state_) :
            underlying_type(state_) {
            this_type::_state.header = StructureSlotHeader::from<trait_type::slot_type>();
        }

        IntegralSlot(mref<StructureSlotState> state_) :
            underlying_type(_STD move(state_)) {
            this_type::_state.header = StructureSlotHeader::from<trait_type::slot_type>();
        }

    public:
        [[nodiscard]] bool validateType() const noexcept override {
            return this_type::_state.header.type == trait_type::slot_type;
        }

    public:
        void enter() override {
            auto* const archive = this_type::_state.root->archive;

            constexpr s64 off = sizeof(trait_type::slot_type);
            archive->seek(this_type::_state.offset + off);
        }

        void leave() override {
            // Note: Integral Slot is used with inferred size type
            /*
            const auto* const archive = this_type::_state.root->archive;

            constexpr s64 off = sizeof(trait_type::slot_type);
            const auto start = this_type::_state.offset + off;

            this_type::_state.header.size = archive->tell() - start;
             */
        }

    public:
        void operator<<(cref<value_type> value_) override {
            auto* const archive = this_type::_state.root->archive;
            (*archive) << value_;
        }

        void operator>>(ref<value_type> value_) override {
            auto* const archive = this_type::_state.root->archive;
            (*archive) >> value_;
        }
    };

    /**/

    template <>
    struct IntegralSlotTraits<bool> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eBool;
    };

    template <>
    struct IntegralSlotTraits<u8> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eU8;
    };

    template <>
    struct IntegralSlotTraits<u16> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eU16;
    };

    template <>
    struct IntegralSlotTraits<u32> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eU32;
    };

    template <>
    struct IntegralSlotTraits<u64> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eU64;
    };

    template <>
    struct IntegralSlotTraits<s8> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eS8;
    };

    template <>
    struct IntegralSlotTraits<s16> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eS16;
    };

    template <>
    struct IntegralSlotTraits<s32> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eS32;
    };

    template <>
    struct IntegralSlotTraits<s64> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eS64;
    };
}
