#pragma once

#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename PackType_ = _STD pair>
    class MapEntryScopedSlot final :
        public ScopedStructureSlot<PackType_<KeyType_, ValueType_>> {
    public:
        using this_type = MapEntryScopedSlot<KeyType_, ValueType_, PackType_>;

        using pack_type = PackType_<KeyType_, ValueType_>;
        using key_type = KeyType_;
        using value_type = ValueType_;

    public:
        MapEntryScopedSlot(cref<ScopedSlotState> state_) :
            ScopedStructureSlot(state_) {
            this_type::ensureEntered(not this_type::_state.isScopedImmutable());
        }

        MapEntryScopedSlot(mref<ScopedSlotState> state_) :
            ScopedStructureSlot(_STD move(state_)) {
            this_type::ensureEntered(not this_type::_state.isScopedImmutable());
        }

        ~MapEntryScopedSlot() override = default;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eMapEntry;
        }

    protected:
        void enter(const bool mutating_) override { }

        void leave(const bool mutating_) override { }

    public:
        [[nodiscard]] const ScopedStructureSlot<KeyType_> getKeySlot() const noexcept {
            return *static_cast<const ptr<ScopedStructureSlot<KeyType_>>>(nullptr);
        }

        [[nodiscard]] ScopedStructureSlot<KeyType_> getKeySlot() noexcept {
            return *static_cast<ptr<ScopedStructureSlot<KeyType_>>>(nullptr);
        }

        [[nodiscard]] const ScopedStructureSlot<ValueType_> getValueSlot() const noexcept {
            return *static_cast<const ptr<ScopedStructureSlot<ValueType_>>>(nullptr);
        }

        [[nodiscard]] ScopedStructureSlot<ValueType_> getValueSlot() noexcept {
            return *static_cast<ptr<ScopedStructureSlot<ValueType_>>>(nullptr);
        }

    public:
        void operator<<(cref<pack_type> object_) override {}

        void operator>>(ref<pack_type> object_) const override {}
    };
}
