#pragma once

#include <type_traits>
#include <concepts>

#include "ScopedStructureSlot.hpp"

#include "StringScopedSlot.hpp"
#include "IntegralScopedSlot.hpp"

namespace ember::engine::serialization {
    class StructIdentifierScopedSlot final :
        public ScopedStructureSlotBase {
    public:
        friend class StructScopedSlot;

    public:
        using this_type = StructIdentifierScopedSlot;

    protected:
        StructIdentifierScopedSlot(cref<ScopedSlotState> state_);

        StructIdentifierScopedSlot(mref<ScopedSlotState> state_);

    public:
        ~StructIdentifierScopedSlot() override;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override;

    protected:
        const non_owning_rptr<ScopedStructureSlotBase> enter() override;

        const non_owning_rptr<ScopedStructureSlotBase> leave() override;

    private:
        [[nodiscard]] const StringScopedSlot intoString() const;

        [[nodiscard]] StringScopedSlot intoString();

        [[nodiscard]] const IntegralScopedSlot<u64> intoU64() const;

        [[nodiscard]] IntegralScopedSlot<u64> intoU64();

    public:
        void operator<<(cref<string> identifier_);

        void operator<<(cref<u64> identifier_);

        template <typename Hasher_> requires
            _STD is_convertible_v<_STD invoke_result_t<Hasher_>, u64> &&
            _STD is_convertible_v<u64, _STD invoke_result_t<Hasher_>>
        void operator>>(cref<_STD invoke_result_t<Hasher_>> rep_) {
            this->operator<<(static_cast<cref<u64>>(rep_));
        }

        template <typename Hasher_>
        void operator<<(cref<string> identifier_) {
            const Hasher_ hasher {};
            this->operator<<<Hasher_>((hasher)(identifier_));
        }

    public:
        void operator>>(ref<string> identifier_);

        void operator>>(ref<u64> identifier_);
    };
}
