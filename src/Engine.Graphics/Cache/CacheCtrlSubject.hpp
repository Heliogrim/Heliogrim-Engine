#pragma once

#include <atomic>
#include <concepts>

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::cache {
	template <std::equality_comparable SubjectType_>
	class CacheCtrlSubject {
	public:
		using this_type = CacheCtrlSubject<SubjectType_>;
		using subject_type = SubjectType_;

	public:
		CacheCtrlSubject() = default;

		CacheCtrlSubject(cref<SubjectType_> subject_, const u16 initialMarks_ = 0uL) noexcept :
			subject(subject_),
			marks(initialMarks_),
			locked() {}

		CacheCtrlSubject(mref<SubjectType_> subject_, const u16 initialMarks_ = 0uL) noexcept :
			subject(std::move(subject_)),
			marks(initialMarks_),
			locked() {}

		CacheCtrlSubject(cref<this_type>) = delete;

		CacheCtrlSubject(mref<this_type> other_) noexcept :
			subject(std::move(other_.subject)),
			// Error: Race-Condition? TODO: Rework !?
			marks(std::move(other_.marks).load()),
			locked() {

			/**
			 * Copy lock state
			 */
			if (other_.locked.test(std::memory_order::consume)) {
				locked.test_and_set(std::memory_order::relaxed);
			}
		}

		~CacheCtrlSubject() noexcept = default;

	public:
		[[nodiscard]] ref<this_type> operator=(cref<this_type>) = delete;

		[[nodiscard]] ref<this_type> operator=(mref<this_type>) noexcept = delete;

	public:
		SubjectType_ subject;
		std::atomic_uint16_t marks;
		std::atomic_flag locked;

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
