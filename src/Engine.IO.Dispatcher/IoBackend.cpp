#include "IoBackend.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/stdafx.h>

#include "BlockingBackend.hpp"
#include "IocpBackend.hpp"

using namespace hg::engine::io;
using namespace hg;

IoCompletion IoBackend::translate(const u32 error_, const u32 bytes_, const u32 size_) noexcept {
	if (error_ == ERROR_SUCCESS) {
		return { bytes_ == size_ ? IoStatus::eDone : IoStatus::eShort, bytes_, 0u };
	}
	if (error_ == ERROR_HANDLE_EOF) {
		return { IoStatus::eShort, bytes_, 0u };
	}
	if (error_ == ERROR_OPERATION_ABORTED) {
		// eCancelled promises an untouched buffer; a partial abort reports what landed.
		return { bytes_ > 0u ? IoStatus::eShort : IoStatus::eCancelled, bytes_, 0u };
	}
	return { IoStatus::eFailed, bytes_, error_ };
}

uptr<IoBackend> IoBackend::make(const IoBackendKind kind_, ref<IoThread> thread_, sink_type sink_) {
	switch (kind_) {
		case IoBackendKind::eBlocking: {
			return make_uptr<BlockingBackend>(std::move(sink_));
		}
		case IoBackendKind::eIocp: {
			break;
		}
	}
	return make_uptr<IocpBackend>(thread_, std::move(sink_));
}
