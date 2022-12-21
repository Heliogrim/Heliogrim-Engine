#pragma once

#include "../Archive/Archive.hpp"

#include "ScopedStructureSlot.hpp"
#include "StructureSlotState.hpp"

#include "SubstitutionSlot.hpp"
#include "StructureSlotTypeTraits.hpp"

#ifdef _DEBUG
#include <Engine.Logging/Logger.hpp>
#endif

namespace ember::engine::serialization {
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
            ScopedStructureSlot<slice_type>(state_) {}

        SliceScopedSlot(mref<ScopedSlotState> state_) :
            ScopedStructureSlot<slice_type>(_STD move(state_)) {}

        ~SliceScopedSlot() override = default;

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eSlice;
        }

    protected:
        const non_owning_rptr<ScopedStructureSlotBase> enter() override {
            return nullptr;
        }

        const non_owning_rptr<ScopedStructureSlotBase> leave() override {
            return nullptr;
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

            auto* archive = this_type::_state.rootState->getArchive();

            (*archive) << StructureSlotType::eSlice;

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

            auto* const archive = this_type::_state.rootState->getArchive();

            /**/

            StructureSlotType storedType = StructureSlotType::eUndefined;
            (*archive) >> storedType;

            #ifdef _DEBUG
            if (storedType != StructureSlotType::eSlice) {
                IM_CORE_WARNF(
                    "Tried to deserialize a `{}` into `{}`",
                    StructureSlotTypeTrait::canonical(storedType),
                    StructureSlotTypeTraits<StructureSlotType::eSlice>::canonical
                );
            }
            #endif

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
}
