#include "BlockingBackend.hpp"

#include <limits>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/stdafx.h>

#include "IoFile.hpp"

using namespace hg::engine::io;
using namespace hg;

BlockingBackend::BlockingBackend(sink_type sink_) :
	_sink(std::move(sink_)) {}

u32 BlockingBackend::openFlags() const noexcept {
	return 0u;
}

u32 BlockingBackend::associate([[maybe_unused]] const ptr<void> handle_) {
	return 0u;
}

void BlockingBackend::issue(ref<const IoRequest> request_, const ptr<void> token_) {
	if (request_.buffer.size() > std::numeric_limits<DWORD>::max()) {
		_sink(token_, { IoStatus::eFailed, 0u, ERROR_INVALID_PARAMETER });
		return;
	}

	// The handle is synchronous; the OVERLAPPED only carries the offset.
	OVERLAPPED at {};
	at.Offset = static_cast<DWORD>(request_.offset & 0xFFFFFFFFuLL);
	at.OffsetHigh = static_cast<DWORD>(request_.offset >> 32uLL);
	const auto size = static_cast<DWORD>(request_.buffer.size());
	const auto file = request_.file->handle();

	DWORD bytes = 0;
	const auto ok = request_.op == IoOp::eRead ?
		::ReadFile(file, request_.buffer.data(), size, &bytes, &at) :
		::WriteFile(file, request_.buffer.data(), size, &bytes, &at);
	_sink(token_, translate(ok ? ERROR_SUCCESS : ::GetLastError(), bytes, size));
}

void BlockingBackend::harvest([[maybe_unused]] ref<const IoPortEntry> entry_) {
	// Nothing is associated with the port, so no packet can carry a request.
	::hg::assertd(false);
}

void BlockingBackend::cancel([[maybe_unused]] const ptr<void> token_) {}

void BlockingBackend::cancelAll() {}
