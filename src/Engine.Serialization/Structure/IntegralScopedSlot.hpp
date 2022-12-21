#pragma once

#include <concepts>
#include <Engine.Common/Types.hpp>

#include "../Archive/Archive.hpp"
#include "ScopedStructureSlot.hpp"
#include "StructureSlotState.hpp"
#include "ScopedSlotGuard.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#include "StructureSlotTypeTraits.hpp"
#endif

namespace ember::engine::serialization {
    template <_STD integral Type_>
    struct IntegralSlotTraits;

    template <_STD integral IntegralType_>
    class IntegralScopedSlot final :
        public ScopedStructureSlot<IntegralType_> {
    public:
        using this_type = IntegralScopedSlot<IntegralType_>;

        using data_type = IntegralType_;
        using trait_type = IntegralSlotTraits<IntegralType_>;

    public:
        IntegralScopedSlot(cref<ScopedSlotState> state_) :
            ScopedStructureSlot<data_type>(state_) {}

        IntegralScopedSlot(mref<ScopedSlotState> state_) :
            ScopedStructureSlot<data_type>(_STD move(state_)) {}

        ~IntegralScopedSlot() override = default;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return trait_type::slot_type;
        }

    protected:
        void enter(const bool mutating_) override { }

        void leave(const bool mutating_) override { }

    public:
        void operator<<(cref<data_type> object_) override {
            const ScopedSlotGuard guard { this, true };

            auto* const archive = this_type::_state.rootState->getArchive();
            (*archive) << trait_type::slot_type;
            (*archive) << object_;
        }

        void operator>>(ref<data_type> object_) const override {

            auto* const archive = this_type::_state.rootState->getArchive();

            /**/

            StructureSlotType storedType { StructureSlotType::eUndefined };
            (*archive) >> storedType;

            #ifdef _DEBUG
            if (storedType != trait_type::slot_type) {
                IM_CORE_WARNF(
                    "Tried to deserialize a `{}` into a `{}`.",
                    StructureSlotTypeTrait::canonical(storedType),
                    StructureSlotTypeTraits<trait_type::slot_type>::canonical
                );
            }
            #endif

            /**/

            (*archive) >> object_;
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
