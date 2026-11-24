#include "IoFile.hpp"

#include <Engine.Common/stdafx.h>

using namespace hg::engine::io;
using namespace hg;

IoFile::IoFile(const ptr<void> handle_, const u32 alignment_) noexcept :
	_handle(handle_),
	_alignment(alignment_) {}

IoFile::~IoFile() {
	if (_handle != nullptr && _handle != INVALID_HANDLE_VALUE) {
		::CloseHandle(_handle);
	}
}

ptr<void> IoFile::handle() const noexcept {
	return _handle;
}

u32 IoFile::alignment() const noexcept {
	return _alignment;
}
