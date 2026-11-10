#pragma once

#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/ThreadSafety.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Meta/LockTag.hpp"

namespace hg::concurrent {
	/**
	 * Drop-in for std::unique_lock without <mutex>. Misuse breaks in debug instead of throwing.
	 */
	template <typename Mutex_>
	class UniqueLock {
	public:
		using this_type = UniqueLock<Mutex_>;
		using mutex_type = Mutex_;

	public:
		UniqueLock() noexcept = default;

		explicit UniqueLock(ref<Mutex_> mtx_) :
			_mtx(&mtx_) {
			lock();
		}

		UniqueLock(ref<Mutex_> mtx_, defer_lock_t) noexcept :
			_mtx(&mtx_) {}

		UniqueLock(ref<Mutex_> mtx_, try_to_lock_t) :
			_mtx(&mtx_) {
			_owns = _mtx->try_lock();
		}

		UniqueLock(ref<Mutex_> mtx_, adopt_lock_t) noexcept :
			_mtx(&mtx_),
			_owns(true) {}

		UniqueLock(ref<const this_type>) = delete;

		UniqueLock(mref<this_type> other_) noexcept :
			_mtx(other_._mtx),
			_owns(other_._owns) {
			other_._mtx = nullptr;
			other_._owns = false;
		}

		~UniqueLock() HG_NO_THREAD_SAFETY_ANALYSIS {
			if (_owns) {
				_mtx->unlock();
			}
		}

	public:
		ref<this_type> operator=(ref<const this_type>) = delete;

		ref<this_type> operator=(mref<this_type> other_) noexcept HG_NO_THREAD_SAFETY_ANALYSIS {
			if (this != &other_) {
				if (_owns) {
					_mtx->unlock();
				}
				_mtx = other_._mtx;
				_owns = other_._owns;
				other_._mtx = nullptr;
				other_._owns = false;
			}
			return *this;
		}

	private:
		ptr<Mutex_> _mtx = nullptr;
		bool _owns = false;

	public:
		void lock() HG_NO_THREAD_SAFETY_ANALYSIS {
			if (_mtx == nullptr || _owns) {
				::hg::breakpoint();
			}
			_mtx->lock();
			_owns = true;
		}

		[[nodiscard]] bool try_lock() HG_NO_THREAD_SAFETY_ANALYSIS {
			if (_mtx == nullptr || _owns) {
				::hg::breakpoint();
			}
			_owns = _mtx->try_lock();
			return _owns;
		}

		void unlock() HG_NO_THREAD_SAFETY_ANALYSIS {
			if (not _owns) {
				::hg::breakpoint();
			}
			_mtx->unlock();
			_owns = false;
		}

		void swap(ref<this_type> other_) noexcept {
			auto tmp = ::hg::move(other_);
			other_ = ::hg::move(*this);
			*this = ::hg::move(tmp);
		}

		[[nodiscard]] ptr<Mutex_> release() noexcept {
			const auto mtx = _mtx;
			_mtx = nullptr;
			_owns = false;
			return mtx;
		}

		[[nodiscard]] ptr<Mutex_> mutex() const noexcept {
			return _mtx;
		}

		[[nodiscard]] bool owns_lock() const noexcept {
			return _owns;
		}

		[[nodiscard]] explicit operator bool() const noexcept {
			return _owns;
		}
	};

	template <typename Mutex_>
	UniqueLock(ref<Mutex_>) -> UniqueLock<Mutex_>;

	template <typename Mutex_, typename Tag_>
	UniqueLock(ref<Mutex_>, Tag_) -> UniqueLock<Mutex_>;
}
