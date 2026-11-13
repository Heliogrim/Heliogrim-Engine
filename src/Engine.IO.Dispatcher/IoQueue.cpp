#include "IoQueue.hpp"

#include <Engine.Common/stdafx.h>

#include "IoSystem.hpp"

using namespace hg::engine::io;
using namespace hg;

struct IoQueue::Op {
	ptr<Batch> batch;
	u32 index;
	bool finished;
};

struct IoQueue::Batch {
	Promise<Vector<IoCompletion>> promise;
	Vector<IoRequest> requests;
	Vector<IoCompletion> results;
	Vector<Op> ops;
	// Requests below this were issued or skipped.
	u32 next = 0;
	u32 open = 0;
	// Cycle with the queue, broken when the batch resolves.
	Arci<IoQueue> owner;
};

IoQueue::IoQueue(ref<IoSystem> system_, const IoPriority priority_) :
	_system(system_),
	_priority(priority_) {}

IoQueue::~IoQueue() = default;

Future<Vector<IoCompletion>> IoQueue::submit(mref<Vector<IoRequest>> requests_) {
	Promise<Vector<IoCompletion>> promise {};
	auto future = promise.future();
	// A rejected post destroys the closure, which breaks the promise.
	_system.thread().postDetached(
		[self = arci_from_this(), promise = std::move(promise), requests = std::move(requests_)]() mutable {
			self->accept(std::move(promise), std::move(requests));
		}
	);
	return future;
}

void IoQueue::cancel(const u64 mask_, const u64 value_) {
	_system.thread().postDetached(
		[self = arci_from_this(), mask_, value_] {
			for (auto& batch : self->_batches) {
				for (auto i = batch->next; i < batch->ops.size(); ++i) {
					auto& op = batch->ops[i];
					if (not op.finished && (batch->requests[i].tag & mask_) == value_) {
						batch->results[i] = { IoStatus::eCancelled, 0u, 0u };
						op.finished = true;
						--batch->open;
					}
				}
			}
			self->resolveReady();
		}
	);
}

IoPriority IoQueue::priority() const noexcept {
	return _priority;
}

void IoQueue::accept(mref<Promise<Vector<IoCompletion>>> promise_, mref<Vector<IoRequest>> requests_) {
	auto batch = std::make_unique<Batch>();
	batch->promise = std::move(promise_);
	batch->requests = std::move(requests_);

	const auto count = static_cast<u32>(batch->requests.size());
	batch->results.assign(count, IoCompletion { IoStatus::eFailed, 0u, 0u });
	batch->ops.resize(count);
	batch->open = count;
	batch->owner = arci_from_this();

	for (u32 i = 0; i < count; ++i) {
		batch->ops[i] = { batch.get(), i, false };
		if (batch->requests[i].compression != IoCompression::eNone || batch->requests[i].file == nullptr) {
			batch->results[i].osError = ERROR_INVALID_PARAMETER;
			batch->ops[i].finished = true;
			--batch->open;
		}
	}

	_batches.push_back(std::move(batch));
	resolveReady();
	_system.activate(arci_from_this());
}

void IoQueue::resolveReady() {
	while (not _batches.empty() && _batches.front()->open == 0) {
		auto batch = std::move(_batches.front());
		_batches.pop_front();
		batch->promise.resolve(std::move(batch->results));
	}
}

ptr<void> IoQueue::next() noexcept {
	for (auto& batch : _batches) {
		while (batch->next < batch->ops.size() && batch->ops[batch->next].finished) {
			++batch->next;
		}
		if (batch->next < batch->ops.size()) {
			return &batch->ops[batch->next++];
		}
	}
	return nullptr;
}

ref<const IoRequest> IoQueue::requestOf(const ptr<void> token_) noexcept {
	const auto* const op = static_cast<const Op*>(token_);
	return op->batch->requests[op->index];
}

void IoQueue::complete(const ptr<void> token_, const IoCompletion completion_) {
	auto* const op = static_cast<Op*>(token_);
	// Local ref: resolving may drop the batch's own reference.
	auto self = op->batch->owner;
	op->batch->results[op->index] = completion_;
	op->finished = true;
	--op->batch->open;
	self->resolveReady();
}
