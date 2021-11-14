#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scheduler/Stage/ScheduleStage.hpp>

namespace ember::engine::ecs::subsystem {

    class StagedComponentSubmit {
    public:
        constexpr StagedComponentSubmit() noexcept;

        constexpr StagedComponentSubmit(_In_ u32 pageIdx_, _In_ non_owning_rptr<void> data_) noexcept;

        StagedComponentSubmit(_In_ mref<StagedComponentSubmit> other_) noexcept;

        StagedComponentSubmit(_In_ cref<StagedComponentSubmit> other_) noexcept;

        ~StagedComponentSubmit() noexcept;

    public:
        ref<StagedComponentSubmit> operator=(_In_ mref<StagedComponentSubmit> other_) noexcept;

        ref<StagedComponentSubmit> operator=(_In_ cref<StagedComponentSubmit> other_) noexcept;

    private:
        u32 _pageIdx;
        non_owning_rptr<void> _data;

    public:
        [[nodiscard]] u32 pageIdx() const noexcept;

        [[nodiscard]] const non_owning_rptr<void> data() const noexcept;

    public:
        constexpr bool operator<(cref<StagedComponentSubmit> other_) const noexcept {

            if (_pageIdx < other_._pageIdx) {
                return true;
            }

            if (_pageIdx > other_._pageIdx) {
                return false;
            }

            return _data < other_._data;
        }

        constexpr bool operator>(cref<StagedComponentSubmit> other_) const noexcept {

            if (_pageIdx > other_._pageIdx) {
                return true;
            }

            if (_pageIdx < other_._pageIdx) {
                return false;
            }

            return _data > other_._data;
        }

        constexpr bool operator==(cref<StagedComponentSubmit> other_) const noexcept {
            return _pageIdx == other_._pageIdx && _data == other_._data;
        }

        constexpr bool operator!=(cref<StagedComponentSubmit> other_) const noexcept {
            return _pageIdx != other_._pageIdx || _data != other_._data;
        }

        constexpr bool operator<=(cref<StagedComponentSubmit> other_) const noexcept {

            if (_pageIdx < other_._pageIdx) {
                return true;
            }

            if (_pageIdx > other_._pageIdx) {
                return false;
            }

            return _data <= other_._data;
        }

        constexpr bool operator>=(cref<StagedComponentSubmit> other_) const noexcept {

            if (_pageIdx > other_._pageIdx) {
                return true;
            }

            if (_pageIdx < other_._pageIdx) {
                return false;
            }

            return _data >= other_._data;
        }
    };
}

namespace std {

    template <>
    struct less<ember::engine::ecs::subsystem::StagedComponentSubmit> {
        [[nodiscard]] FORCE_INLINE bool operator()(
            ember::cref<ember::engine::ecs::subsystem::StagedComponentSubmit> left_,
            ember::cref<ember::engine::ecs::subsystem::StagedComponentSubmit> right_) const noexcept {

            if (left_.pageIdx() < right_.pageIdx()) {
                return true;
            }

            if (left_.pageIdx() > right_.pageIdx()) {
                return false;
            }

            return left_.data() < right_.data();
        }
    };

}
