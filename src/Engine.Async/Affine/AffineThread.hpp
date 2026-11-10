#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/UniqueFn.hpp>

#include "../Future/Future.hpp"
#include "../Future/Promise.hpp"
#include "../Future/Send.hpp"

namespace hg::engine::async {
	// Called from any thread to make the loop return from its OS wait; UniqueFn is call-once, so a repeatable callable is used.
	using AffineWaker = std::move_only_function<void() const>;

	// Destroy only after stop() and after every in-flight post/postDetached call has returned.
	class AffineThread final {
	public:
		AffineThread(string name_, mref<AffineWaker> waker_);

		AffineThread(const AffineThread&) = delete;

		~AffineThread();

	public:
		AffineThread& operator=(const AffineThread&) = delete;

	public:
		/**
		 * Loop contract (set the waker first if the constructor got none), exactly this shape:
		 *   for (;;) { drain(); if (stopping()) { drain(); return; } os_wait(); }
		 * Checking stopping() only after a drain closes the lost wake against stop() racing setWaker.
		 * Must not be called after stop(), and must happen-before any stop() on another thread.
		 */
		void start(mref<UniqueFn<void(ref<AffineThread>)>> loop_);

		void drain();

		[[nodiscard]] bool stopping() const noexcept;

		// Idempotent and thread-safe: every caller returns only after the loop thread was joined. Before start() it only closes the inbox.
		void stop();

	public:
		template <typename Fn_> requires Send<std::remove_cvref_t<Fn_>>
		[[nodiscard]] Future<std::invoke_result_t<std::decay_t<Fn_>>> post(Fn_&& fn_) {
			using result_type = std::invoke_result_t<std::decay_t<Fn_>>;

			Promise<result_type> source {};
			auto future = source.future();
			if (not push(
				Item {
					[promise = std::move(source), fn = std::forward<Fn_>(fn_)]() mutable {
						if constexpr (std::is_void_v<result_type>) {
							std::invoke(std::move(fn));
							promise.resolve();
						} else {
							promise.resolve(std::invoke(std::move(fn)));
						}
					}
				}
			)) {
				// The closed inbox destroyed the item and so dropped its promise; report the rejection on a fresh one.
				Promise<result_type> rejected {};
				future = rejected.future();
				rejected.reject(Broken { Broken::Reason::eRejected });
			}
			return future;
		}

		template <typename Fn_> requires Send<std::remove_cvref_t<Fn_>>
		void postDetached(Fn_&& fn_) {
			if (not push(Item { [fn = std::forward<Fn_>(fn_)]() mutable { std::invoke(std::move(fn)); } })) {
				warnRejected();
			}
		}

	public:
		// Affine thread only (see the loop contract on start).
		void setWaker(mref<AffineWaker> waker_);

		[[nodiscard]] u64 depth() const noexcept;

		[[nodiscard]] static ptr<AffineThread> current() noexcept;

		[[nodiscard]] bool isCurrent() const noexcept;

		#ifdef _DEBUG
		[[nodiscard]] u64 warningsIssued() const noexcept;
		#endif

	private:
		using Item = UniqueFn<void(), 128>;

		// Nodes are aligned to max_align_t, so bit 0 of the head is free for the closed tag.
		static constexpr std::uintptr_t closed_bit = 1uLL;
		static constexpr u64 depth_warning = 1024uLL;
		static constexpr u64 depth_rearm = 512uLL;

		struct Node {
			Item fn;
			ptr<Node> next = nullptr;
		};

	private:
		[[nodiscard]] bool push(mref<Item> item_);

		void wake() const;

		void warnRejected();

	private:
		string _name;
		std::thread _thread;

		std::atomic<std::uintptr_t> _head { 0uLL };
		ptr<Node> _pending = nullptr;

		std::atomic<u64> _depth { 0uLL };
		std::atomic<u64> _warnings { 0uLL };
		std::atomic_bool _depthArmed { true };
		std::atomic_bool _joining { false };
		std::atomic_bool _joined { false };
		bool _started = false;
		std::atomic_bool _rejectWarned { false };

		std::atomic<ptr<const AffineWaker>> _waker { nullptr };
		// Every waker ever set stays alive until destruction, since a producer may still be calling an older one.
		std::vector<std::unique_ptr<AffineWaker>> _wakers;
	};
}
