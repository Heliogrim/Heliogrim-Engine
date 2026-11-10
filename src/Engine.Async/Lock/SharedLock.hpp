#pragma once

#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/ThreadSafety.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Meta/LockTag.hpp"

namespace hg::concurrent {
	/**
	 * Drop-in for std::shared_lock without <shared_mutex>. Misuse breaks in debug instead of throwing.
	 */
	template <typename Mutex_>
	class SharedLock {
	public:
		using this_type = SharedLock<Mutex_>;
		using mutex_type = Mutex_;

	public:
		SharedLock() noexcept = default;

		explicit SharedLock(ref<Mutex_> mtx_) :
			_mtx(&mtx_) {
			lock();
		}

		SharedLock(ref<Mutex_> mtx_, defer_lock_t) noexcept :
			_mtx(&mtx_) {}

		SharedLock(ref<Mutex_> mtx_, try_to_lock_t) :
			_mtx(&mtx_) {
			_owns = _mtx->try_lock_shared();
		}

		SharedLock(ref<Mutex_> mtx_, adopt_lock_t) noexcept :
			_mtx(&mtx_),
			_owns(true) {}

		SharedLock(cref<this_type>) = delete;

		SharedLock(mref<this_type> other_) noexcept :
			_mtx(other_._mtx),
			_owns(other_._owns) {
			other_._mtx = nullptr;
			other_._owns = false;
		}

		~SharedLock() HG_NO_THREAD_SAFETY_ANALYSIS {
			if (_owns) {
				_mtx->unlock_shared();
			}
		}

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type> other_) noexcept HG_NO_THREAD_SAFETY_ANALYSIS {
			if (this != &other_) {
				if (_owns) {
					_mtx->unlock_shared();
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
			_mtx->lock_shared();
			_owns = true;
		}

		[[nodiscard]] bool try_lock() HG_NO_THREAD_SAFETY_ANALYSIS {
			if (_mtx == nullptr || _owns) {
				::hg::breakpoint();
			}
			_owns = _mtx->try_lock_shared();
			return _owns;
		}

		void unlock() HG_NO_THREAD_SAFETY_ANALYSIS {
			if (not _owns) {
				::hg::breakpoint();
			}
			_mtx->unlock_shared();
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
	SharedLock(ref<Mutex_>) -> SharedLock<Mutex_>;

	template <typename Mutex_, typename Tag_>
	SharedLock(ref<Mutex_>, Tag_) -> SharedLock<Mutex_>;
}
