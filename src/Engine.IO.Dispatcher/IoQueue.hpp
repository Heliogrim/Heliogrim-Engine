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
		IoQueue(ref<IoSystem> system_, const IoPriority priority_);

		~IoQueue();

	public:
		[[nodiscard]] Future<Vector<IoCompletion>> submit(mref<Vector<IoRequest>> requests_);

		// Cancels requests with `(tag & mask_) == value_`: not yet issued ones complete as eCancelled, issued ones go
		// through CancelIoEx and complete as eCancelled (no bytes) or eShort/eDone.
		void cancel(const u64 mask_, const u64 value_);

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

		// Io thread: cancels unfinished requests whose tag matches; issued ones via the backend.
		void cancelMatching(const u64 mask_, const u64 value_);

		// Io thread: the next request in FIFO order as a token, or nullptr.
		[[nodiscard]] ptr<void> next() noexcept;

		[[nodiscard]] static ref<const IoRequest> requestOf(const ptr<void> token_) noexcept;

		static void complete(const ptr<void> token_, const IoCompletion completion_);
	};
}
