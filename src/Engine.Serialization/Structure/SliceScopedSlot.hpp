#pragma once

#include "ScopedStructureSlot.hpp"
#include "SubstitutionSlot.hpp"
#include "SliceSlot.hpp"

namespace hg::engine::serialization {
    template <typename ValueType_, template <typename...> typename SliceType_>
    class SliceScopedSlot final :
        TypeScopedSlot<SliceType_<ValueType_>> {
    public:
        using this_type = SliceScopedSlot<ValueType_, SliceType_>;
        using underlying_type = TypeScopedSlot<SliceType_<ValueType_>>;

        using value_type = typename underlying_type::value_type;

    public:
        SliceScopedSlot(mref<ScopedSlotState> scopedState_, mref<StructureSlotState> state_) :
            underlying_type(_STD move(scopedState_), make_sptr<SliceSlot<ValueType_, SliceType_>>(_STD move(state_))) {}

        ~SliceScopedSlot() override = default;

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

    #if FALSE
    template <typename ValueType_, template <typename...> typename SliceType_>
    class SliceScopedSlot final :
        public ScopedStructureSlot<SliceType_<ValueType_>> {
    public:
        using this_type = SliceScopedSlot<ValueType_, SliceType_>;

        using slice_type = SliceType_<ValueType_>;
        using data_type = ValueType_;

        using scoped_entry_type = typename SubstitutionSlot<ValueType_>::type;

    public:
        SliceScopedSlot(cref<ScopedSlotState> state_) :
            ScopedStructureSlot<slice_type>(state_) {
            this_type::ensureEntered(not this_type::_state.isScopedImmutable());
        }

        SliceScopedSlot(mref<ScopedSlotState> state_) :
            ScopedStructureSlot<slice_type>(_STD move(state_)) {
            this_type::ensureEntered(not this_type::_state.isScopedImmutable());
        }

        ~SliceScopedSlot() override = default;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eSlice;
        }

    protected:
        void enter(const bool mutating_) override {

            ScopedStructureSlot<slice_type>::enter(mutating_);

            /**/

            if (not mutating_ && not this_type::_state.size) {

                auto* archive = this_type::_state.rootState->getArchive();
                archive->seek(this_type::_state.offset);

                StructureSlotType storedType = StructureSlotType::eUndefined;
                (*archive) >> storedType;

    #ifdef _DEBUG
                if (storedType != StructureSlotType::eSlice) {
                    IM_CORE_WARNF(
                        "Tried to deserialize a `{}` into a `{}`.",
                        StructureSlotTypeTrait::canonical(storedType),
                        StructureSlotTypeTraits<StructureSlotType::eSlice>::canonical
                    );
                }
    #endif

                (*archive) >> this_type::_state.size;
                return;
            }

            /**/

            if (mutating_ && not this_type::_state.size) {
                auto* archive = this_type::_state.rootState->getArchive();
                (*archive) << StructureSlotType::eSlice;
                (*archive) << 0ui64;
            }
        }

        void leave(const bool mutating_) override {

            ScopedStructureSlot<slice_type>::leave(mutating_);

            /**/

            if (not mutating_) {
                return;
            }

            /**/

            auto* archive = this_type::_state.rootState->getArchive();
            archive->seek(this_type::_state.offset);

            (*archive) << StructureSlotType::eSlice;
            (*archive) << (this_type::_state.size - sizeof(StructureSlotType::eSlice) - sizeof(u64));

            archive->seek(this_type::_state.offset + this_type::_state.size);
        }

    public:
        [[nodiscard]] const scoped_entry_type getSliceEntry(const size_t index_) const {
            return *static_cast<const ptr<scoped_entry_type>>(nullptr);
        }

        [[nodiscard]] size_t getSliceSize() const noexcept {
            return ~0;
        }

    public:
        void operator<<(cref<slice_type> object_) override {

            const ScopedSlotGuard guard { this, true };
            auto* archive = this_type::_state.rootState->getArchive();

            /* Write dummy / placeholder to archive and safe position */
            const auto countMarker = archive->tell();
            u64 count = 0ui64;
            (*archive) << count;

            /**/

            auto begin = _STD ranges::begin(object_);
            const auto end = _STD ranges::end(object_);

            for (auto it = begin; it != end; ++it) {

                ScopedSlotState state {
                    ScopedSlotStateFlag::eUndefined,
                    *this,
                    this_type::_state.rootState
                };
                scoped_entry_type slot { _STD move(state) };

                /**/

                slot << *it;
                ++count;
            }

            /**/

            const auto cursor = archive->tell();
            archive->seek(countMarker);
            (*archive) << count;

            /**/

            archive->seek(cursor);
        }

        void operator>>(ref<slice_type> object_) const override {

            const ScopedSlotGuard guard { this, false };
            auto* const archive = this_type::_state.rootState->getArchive();

            /**/

            u64 storedCount { 0ui64 };
            (*archive) >> storedCount;

            assert(storedCount != ~0ui64);

            /**/

            // TODO: This might be not available depending on the sub type and container type
            object_.resize(storedCount);

            auto begin = _STD ranges::begin(object_);
            const auto end = _STD ranges::end(object_);

            for (auto it = begin; it != end; ++it) {

                ScopedSlotState state {
                    ScopedSlotStateFlag::eImmutable,
                    *this,
                    this_type::_state.rootState
                };
                scoped_entry_type slot { _STD move(state) };

                /**/

                slot >> *it;
            }

        }
    };
    #endif
}
