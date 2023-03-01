#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "StructurePath.hpp"
#include "__fwd.hpp"

#include "StructureSlotType.hpp"
#include "StructureSlotState.hpp"
#include "SlotOpResult.hpp"

namespace hg::engine::serialization {
    class __declspec(novtable) StructureSlotBase :
        public _STD enable_shared_from_this<StructureSlotBase> {
    public:
        using this_type = StructureSlotBase;

    protected:
        StructureSlotBase(cref<StructureSlotState> state_);

        StructureSlotBase(mref<StructureSlotState> state_);

    public:
        virtual ~StructureSlotBase() = default;

    protected:
        StructureSlotState _state;

    public:
        [[nodiscard]] ref<StructureSlotState> getState() noexcept {
            return _state;
        }

        [[nodiscard]] cref<StructureSlotHeader> getSlotHeader() const noexcept;

        [[nodiscard]] ref<StructureSlotHeader> getSlotHeader() noexcept;

        [[nodiscard]] virtual StructureSlotType getSlotType() const noexcept;

    public:
        [[nodiscard]] virtual bool validateType() const noexcept = 0;

    public:
        virtual void writeHeader();

        virtual void readHeader();

    public:
        virtual void enter();

        virtual void leave();

        virtual void feedback(const non_owning_rptr<const StructureSlotBase> other_);
    };

    template <typename ValueType_>
    class __declspec(novtable) TypedStructureSlotBase :
        public StructureSlotBase {
    public:
        using this_type = TypedStructureSlotBase<ValueType_>;
        using value_type = _STD decay_t<ValueType_>;

    protected:
        TypedStructureSlotBase(cref<StructureSlotState> state_) :
            StructureSlotBase(state_) {}

        TypedStructureSlotBase(mref<StructureSlotState> state_) :
            StructureSlotBase(_STD move(state_)) {}

    public:
        virtual void operator<<(cref<value_type> value_) = 0;

        virtual SlotOpResult operator>>(ref<value_type> value_) = 0;
    };
}
