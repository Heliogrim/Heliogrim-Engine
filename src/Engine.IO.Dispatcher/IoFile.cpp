#include "IoFile.hpp"

#include <Engine.Common/stdafx.h>

using namespace hg::engine::io;
using namespace hg;

IoFile::IoFile(const ptr<void> handle_) noexcept :
	_handle(handle_) {}

IoFile::~IoFile() {
	if (_handle != nullptr && _handle != INVALID_HANDLE_VALUE) {
		::CloseHandle(_handle);
	}
}

ptr<void> IoFile::handle() const noexcept {
	return _handle;
}
