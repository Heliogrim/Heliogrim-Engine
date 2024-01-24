#pragma once

#include "StructureSlot.hpp"

namespace hg::engine::serialization {
    template <std::floating_point Type_>
    struct FloatSlotTraits;

    template <std::floating_point FloatType_>
    class FloatSlot :
        public TypedStructureSlotBase<FloatType_> {
    public:
        using this_type = FloatSlot<FloatType_>;
        using underlying_type = TypedStructureSlotBase<FloatType_>;

        using typename underlying_type::value_type;
        using trait_type = FloatSlotTraits<FloatType_>;

    public:
        FloatSlot(cref<StructureSlotState> state_) :
            underlying_type(state_) {
            this_type::_state.header = StructureSlotHeader::from<trait_type::slot_type>();
        }

        FloatSlot(mref<StructureSlotState> state_) :
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
            auto* const archive = this_type::_state.root->archive;

            constexpr s64 off = sizeof(trait_type::slot_type);
            archive->seek(this_type::_state.offset + off);
        }

    public:
        void operator<<(cref<value_type> value_) override {
            auto* const archive = this_type::_state.root->archive;
            (*archive) << value_;
        }

        SlotOpResult operator>>(ref<value_type> value_) override {
            auto* const archive = this_type::_state.root->archive;
            (*archive) >> value_;
            return SlotOpResult::eSuccess;
        }
    };

    /**/

    template <>
    struct FloatSlotTraits<float> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eFloat;
    };

    template <>
    struct FloatSlotTraits<double> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eDouble;
    };

    template <>
    struct FloatSlotTraits<long double> {
        inline constexpr static StructureSlotType slot_type = StructureSlotType::eDouble;
    };
}
