#pragma once

#include "StructureSlot.hpp"
#include "ScopedSlotState.hpp"

namespace ember::engine::serialization {
    class __declspec(novtable) ScopedStructureSlotBase :
        public StructureSlotBase {
    public:
        template <typename>
        friend class ScopedStructureSlot;

    public:
        using this_type = ScopedStructureSlotBase;

    protected:
        ScopedStructureSlotBase(cref<ScopedSlotState> state_);

        ScopedStructureSlotBase(mref<ScopedSlotState> state_);

    public:
        ScopedStructureSlotBase(cref<this_type>) = delete;

        ScopedStructureSlotBase(mref<this_type>) noexcept = delete;

        ~ScopedStructureSlotBase() override;

    public:
        ptr<void> operator new(size_t, ptr<void>) = delete;

    protected:
        ScopedSlotState _state;

    public:
        [[nodiscard]] cref<ScopedSlotState> getState() const noexcept;

        [[nodiscard]] bool emptyState() const noexcept;

    public:
        [[nodiscard]] string_view getSlotName() const noexcept;

        void setSlotName(const string_view name_);

    public:
        virtual void subStateEnter(cref<ScopedStructureSlotBase> subState_) const;

        virtual void subStateLeave(cref<ScopedStructureSlotBase> subState_) const;
    };

    template <typename DataType_>
    class ScopedStructureSlot :
        public ScopedStructureSlotBase {
    public:
        using this_type = ScopedStructureSlot<DataType_>;

        using data_type = DataType_;

    protected:
        ScopedStructureSlot(cref<ScopedSlotState> state_) :
            ScopedStructureSlotBase(state_) {}

        ScopedStructureSlot(mref<ScopedSlotState> state_) :
            ScopedStructureSlotBase(_STD move(state_)) {}

    public:
        ~ScopedStructureSlot() override = default;

    public:
        virtual void operator<<(cref<data_type> object_) = 0;

        virtual void operator>>(ref<data_type> object_) const = 0;
    };
}
