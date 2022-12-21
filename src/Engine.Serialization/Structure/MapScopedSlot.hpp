#pragma once

#include "__fwd.hpp"
#include "ScopedStructureSlot.hpp"

namespace ember::engine::serialization {
    template <typename KeyType_, typename ValueType_, template <typename, typename...> typename MapType_>
    class MapScopedSlot final :
        public ScopedStructureSlot<MapType_<KeyType_, ValueType_>> {
    public:
        using this_type = MapScopedSlot<KeyType_, ValueType_, MapType_>;

        using map_type = MapType_<KeyType_, ValueType_>;
        using key_type = KeyType_;
        using value_type = ValueType_;

        using scoped_entry_type = MapEntryScopedSlot<KeyType_, ValueType_>;

    public:
        MapScopedSlot(cref<ScopedSlotState> state_) :
            ScopedStructureSlot<map_type>(state_) {}

        MapScopedSlot(mref<ScopedSlotState> state_) :
            ScopedStructureSlot<map_type>(_STD move(state_)) {}

        ~MapScopedSlot() override = default;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eMap;
        }

    protected:
        const non_owning_rptr<ScopedStructureSlotBase> enter() override {
            return nullptr;
        }

        const non_owning_rptr<ScopedStructureSlotBase> leave() override {
            return nullptr;
        }

    public:
        [[nodiscard]] size_t getMapSize() const noexcept {
            return ~0;
        }

    public:
        [[nodiscard]] const scoped_entry_type getMapEntry(const size_t index_) const {
            return *static_cast<const ptr<scoped_entry_type>>(nullptr);
        }

        [[nodiscard]] scoped_entry_type getMapEntry(const size_t index_) {
            return *static_cast<ptr<scoped_entry_type>>(nullptr);
        }

    public:
        void operator<<(cref<map_type> object_) override {}

        void operator>>(ref<map_type> object_) const override {}
    };
}
