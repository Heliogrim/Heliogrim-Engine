#pragma once

#include <atomic>
#include <concepts>

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx::cache {
    template <std::totally_ordered SubjectType_>
    class CacheCtrlSubject {
    public:
        using this_type = CacheCtrlSubject<SubjectType_>;
        using subject_type = SubjectType_;

    public:
        CacheCtrlSubject() = default;

        CacheCtrlSubject(cref<SubjectType_> subject_, const u16 initialMarks_ = 0ui16) noexcept :
            subject(subject_),
            marks(initialMarks_),
            locked() { }

        CacheCtrlSubject(mref<SubjectType_> subject_, const u16 initialMarks_ = 0ui16) noexcept :
            subject(_STD move(subject_)),
            marks(initialMarks_),
            locked() { }

        CacheCtrlSubject(cref<this_type>) = delete;

        CacheCtrlSubject(mref<this_type> other_) noexcept :
            subject(_STD move(other_.subject)),
            marks(_STD move(other_.marks)),
            locked() {

            /**
             * Copy lock state
             */
            if (other_.locked.test(_STD memory_order::consume)) {
                locked.test_and_set(_STD memory_order::relaxed);
            }
        }

        ~CacheCtrlSubject() noexcept = default;

    public:
        [[nodiscard]] ref<this_type> operator=(cref<this_type>) = delete;

        [[nodiscard]] ref<this_type> operator=(mref<this_type>) noexcept = delete;

    public:
        SubjectType_ subject;
        _STD atomic_uint16_t marks;
        _STD atomic_flag locked;

    public:
        [[nodiscard]] bool operator==(cref<this_type> other_) const noexcept {
            return subject == other_.subject;
        }

        [[nodiscard]] bool operator==(cref<SubjectType_> subject_) const noexcept {
            return subject == subject_;
        }

        [[nodiscard]] bool operator!=(cref<this_type> other_) const noexcept {
            return subject != other_.subject;
        }

        [[nodiscard]] bool operator!=(cref<SubjectType_> subject_) const noexcept {
            return subject == subject_;
        }
    };
}
