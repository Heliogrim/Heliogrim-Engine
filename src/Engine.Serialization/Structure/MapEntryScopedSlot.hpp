#pragma once

#include "ScopedStructureSlot.hpp"
#include "MapEntrySlot.hpp"

namespace hg::engine::serialization {
    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename PackType_ = _STD pair>
    class MapEntryScopedSlot final :
        TypeScopedSlot<PackType_<KeyType_, ValueType_>> {
    public:
        using this_type = MapEntryScopedSlot<KeyType_, ValueType_, PackType_>;
        using underlying_type = TypeScopedSlot<PackType_<KeyType_, ValueType_>>;

        using value_type = typename underlying_type::value_type;

    public:
        MapEntryScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
            TypeScopedSlot(
                _STD move(scopedState_),
                make_sptr<MapEntrySlot<KeyType_, ValueType_, PackType_>>(_STD move(state_))
            ) {}

        ~MapEntryScopedSlot() override = default;

    public:
        void operator<<(cref<value_type> value_) override {
            const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eWrite };
            (*underlying_type::slot()) << value_;
        }

        void operator>>(ref<value_type> value_) const override {
            const ScopedSlotGuard guard { this, ScopedSlotGuardMode::eRead };
            (*underlying_type::slot()) >> value_;
        }
    };
}
