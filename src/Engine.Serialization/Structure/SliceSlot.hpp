#pragma once

#include <cassert>

#include "StructureSlot.hpp"
#include "SubstitutionSlot.hpp"

namespace ember {
    template <class Type_>
    concept HasResizeFnc = requires(Type_ obj_, u64 count_) {
        { obj_.resize(count_) };
    };
}

namespace ember::engine::serialization {
    template <typename ValueType_, template <typename...> typename SliceType_>
    class SliceSlot final :
        public TypedStructureSlotBase<SliceType_<ValueType_>> {
    public:
        using this_type = SliceSlot<ValueType_, SliceType_>;
        using underlying_type = TypedStructureSlotBase<SliceType_<ValueType_>>;

        using value_type = typename underlying_type::value_type;

        using entry_type = typename SubstitutionSlot<ValueType_>::type;
        using slice_type = value_type;
        using slice_value_type = _STD decay_t<ValueType_>;

    public:
        SliceSlot(mref<StructureSlotState> state_) :
            underlying_type(_STD move(state_)) {
            this_type::_state.header = StructureSlotHeader::from<StructureSlotType::eSlice>();
        }

        ~SliceSlot() override {
            if (this_type::_state.flags & StructureSlotStateFlag::eDirty) {
                this_type::leave();
            }
        }

    public:
        [[nodiscard]] StructureSlotType getSlotType() const noexcept override {
            return StructureSlotType::eSlice;
        }

        [[nodiscard]] bool validateType() const noexcept override {
            return this_type::_state.header.type == StructureSlotType::eSlice;
        }

    public:
        void operator<<(cref<value_type> value_) override {

            auto* const archive = this_type::_state.root->archive;

            /* Write dummy / placeholder to archive and safe position */
            const auto countMarker = archive->tell();
            u64 count = 0ui64;
            (*archive) << count;

            /**/

            auto begin = _STD ranges::begin(value_);
            const auto end = _STD ranges::end(value_);

            constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
            constexpr s64 ioff = sizeof(count);

            s64 offset = this_type::_state.offset + off + ioff;

            for (auto it = begin; it != end; ++it, ++count) {

                StructureSlotState state {
                    StructureSlotStateFlags { StructureSlotStateFlag::eMutable } | StructureSlotStateFlag::eDirty,
                    offset,
                    StructureSlotHeader::from<StructureSlotType::eUndefined>(),
                    this_type::shared_from_this(),
                    this_type::_state.root
                };

                entry_type slot { _STD move(state) };

                /**/

                slot.writeHeader();
                slot.enter();

                slot << *it;

                slot.leave();
                slot.writeHeader();

                /**/

                offset += slot.getSlotHeader().size;
                offset += sizeof(StructureSlotHeader::type);

                if (not slot.getSlotHeader().sizeInferred) {
                    offset += sizeof(StructureSlotHeader::size);
                }
            }

            /**/

            const auto start = this_type::_state.offset + off;
            this_type::_state.header.size = offset - start;

            /**/

            const auto cursor = archive->tell();
            archive->seek(countMarker);
            (*archive) << count;

            /**/

            archive->seek(cursor);
        }

        SlotOpResult operator>>(ref<value_type> value_) override {

            if (this_type::_state.header.size <= 0) {
                return SlotOpResult::eFailed;
            }

            /**/

            auto* const archive = this_type::_state.root->archive;

            /**/

            u64 storedCount = ~0ui64;
            (*archive) >> storedCount;

            assert(storedCount != ~0ui64);

            /**/

            // TODO: if constexpr (_STD is_constructible_v<slice_value_type, ...>)

            if constexpr (HasResizeFnc<value_type>) {
                value_.resize(storedCount);
            } else {
                assert(value_.size() >= storedCount);
            }

            auto begin = _STD ranges::begin(value_);
            const auto end = _STD ranges::end(value_);

            constexpr s64 off = sizeof(StructureSlotHeader::type) + sizeof(StructureSlotHeader::size);
            constexpr s64 ioff = sizeof(storedCount);

            s64 offset = this_type::_state.offset + off + ioff;

            auto it = begin;
            u64 i = 0;
            while (i < storedCount) {

                StructureSlotState state {
                    StructureSlotStateFlag::eImmutable,
                    offset,
                    StructureSlotHeader::from<StructureSlotType::eUndefined>(),
                    this_type::shared_from_this(),
                    this_type::_state.root
                };

                entry_type slot { _STD move(state) };

                /**/

                slot.readHeader();
                slot.enter();

                slot >> (*it);

                slot.leave();

                /**/

                offset += slot.getSlotHeader().size;
                offset += sizeof(StructureSlotHeader::type);

                if (not slot.getSlotHeader().sizeInferred) {
                    offset += sizeof(StructureSlotHeader::size);
                }

                /**/

                ++it;
                ++i;
            }

            /**/

            return SlotOpResult::eSuccess;
        }
    };
}
