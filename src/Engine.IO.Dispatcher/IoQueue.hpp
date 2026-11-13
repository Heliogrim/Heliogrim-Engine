#pragma once

#include <Engine.Async/Future/Future.hpp>
#include <Engine.Async/Future/Promise.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Deque.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "IoTypes.hpp"

namespace hg::engine::io {
	class IoSystem;

	/**
	 * FIFO of batches: a batch resolves once all its requests completed and every earlier batch resolved.
	 * Queue state is only touched on the io thread; the public methods post there.
	 */
	class IoQueue final :
		public ArcFromThis<IoQueue> {
	public:
		friend class IoSystem;

	public:
		IoQueue(ref<IoSystem> system_, IoPriority priority_);

		~IoQueue();

	public:
		[[nodiscard]] Future<Vector<IoCompletion>> submit(mref<Vector<IoRequest>> requests_);

		// Completes not yet issued requests with `(tag & mask_) == value_` as eCancelled.
		void cancel(u64 mask_, u64 value_);

		[[nodiscard]] IoPriority priority() const noexcept;

	private:
		struct Op;
		struct Batch;

	private:
		ref<IoSystem> _system;
		IoPriority _priority;
		bool _listed = false;
		Deque<uptr<Batch>> _batches;

	private:
		void accept(mref<Promise<Vector<IoCompletion>>> promise_, mref<Vector<IoRequest>> requests_);

		void resolveReady();

		// Io thread: the next request in FIFO order as a token, or nullptr.
		[[nodiscard]] ptr<void> next() noexcept;

		[[nodiscard]] static ref<const IoRequest> requestOf(ptr<void> token_) noexcept;

		static void complete(ptr<void> token_, IoCompletion completion_);
	};
}
