#include "IocpBackend.hpp"

#include <limits>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/stdafx.h>

#include "BlockingBackend.hpp"
#include "IoFile.hpp"

using namespace hg::engine::io;
using namespace hg;

namespace {
	struct Slot :
		OVERLAPPED {
		ptr<void> token;
		HANDLE file;
		DWORD size;
	};

	void cancelSlot(const ptr<void> slot_) {
		auto* const slot = static_cast<Slot*>(slot_);
		// ERROR_NOT_FOUND: the request already finished and its packet is queued.
		::CancelIoEx(slot->file, slot);
	}
}

IoCompletion IoBackend::translate(const u32 error_, const u32 bytes_, const u32 size_) noexcept {
	if (error_ == ERROR_SUCCESS) {
		return { bytes_ == size_ ? IoStatus::eDone : IoStatus::eShort, bytes_, 0u };
	}
	if (error_ == ERROR_HANDLE_EOF) {
		return { IoStatus::eShort, bytes_, 0u };
	}
	if (error_ == ERROR_OPERATION_ABORTED) {
		return { IoStatus::eCancelled, bytes_, 0u };
	}
	return { IoStatus::eFailed, bytes_, error_ };
}

uptr<IoBackend> IoBackend::make(const IoBackendKind kind_, ref<IoThread> thread_, sink_type sink_) {
	switch (kind_) {
		case IoBackendKind::eBlocking: {
			return std::make_unique<BlockingBackend>(std::move(sink_));
		}
		case IoBackendKind::eIocp:
		default: {
			return std::make_unique<IocpBackend>(thread_, std::move(sink_));
		}
	}
}

IocpBackend::IocpBackend(ref<IoThread> thread_, sink_type sink_) :
	_thread(thread_),
	_sink(std::move(sink_)) {}

u32 IocpBackend::openFlags() const noexcept {
	return FILE_FLAG_OVERLAPPED;
}

u32 IocpBackend::associate(const ptr<void> handle_) {
	return ::CreateIoCompletionPort(handle_, _thread.port(), 0, 0) != nullptr ? 0u : ::GetLastError();
}

void IocpBackend::issue(ref<const IoRequest> request_, const ptr<void> token_) {
	if (request_.buffer.size() > std::numeric_limits<DWORD>::max()) {
		_sink(token_, { IoStatus::eFailed, 0u, ERROR_INVALID_PARAMETER });
		return;
	}

	auto* const slot = new Slot {};
	slot->Offset = static_cast<DWORD>(request_.offset & 0xFFFFFFFFuLL);
	slot->OffsetHigh = static_cast<DWORD>(request_.offset >> 32uLL);
	slot->token = token_;
	slot->file = request_.file->handle();
	slot->size = static_cast<DWORD>(request_.buffer.size());

	const auto ok = request_.op == IoOp::eRead ?
		::ReadFile(slot->file, request_.buffer.data(), slot->size, nullptr, slot) :
		::WriteFile(slot->file, request_.buffer.data(), slot->size, nullptr, slot);

	// Success and pending both queue a packet; only a synchronous failure never will.
	const auto error = ok ? ERROR_SUCCESS : ::GetLastError();
	if (ok || error == ERROR_IO_PENDING) {
		_slots.emplace(token_, slot);
		return;
	}
	delete slot;
	_sink(token_, translate(error, 0, static_cast<DWORD>(request_.buffer.size())));
}

void IocpBackend::harvest(ref<const IoPortEntry> entry_) {
	const auto& entry = reinterpret_cast<const OVERLAPPED_ENTRY&>(entry_);
	::hg::assertd(entry.lpOverlapped != nullptr);
	auto* const slot = static_cast<Slot*>(entry.lpOverlapped);

	DWORD bytes = 0;
	DWORD error = ERROR_SUCCESS;
	if (not ::GetOverlappedResult(slot->file, slot, &bytes, FALSE)) {
		error = ::GetLastError();
	}

	const auto token = slot->token;
	const auto size = slot->size;
	_slots.erase(token);
	delete slot;
	_sink(token, translate(error, bytes, size));
}

void IocpBackend::cancel(const ptr<void> token_) {
	if (const auto it = _slots.find(token_); it != _slots.end()) {
		cancelSlot(it->second);
	}
}

void IocpBackend::cancelAll() {
	for (const auto& [token, slot] : _slots) {
		cancelSlot(slot);
	}
}
