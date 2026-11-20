#include <cstring>

import Engine.Resource.Archive;
import std;

using namespace hg::engine::resource;
using namespace hg;

MemoryReadonlyArchive::MemoryReadonlyArchive(ref<std::span<_::byte, std::dynamic_extent>> bytes_) :
	MemoryBaseArchive(),
	_bytes(bytes_),
	_limit(-1) {}

MemoryReadonlyArchive::MemoryReadonlyArchive(ref<std::span<_::byte, std::dynamic_extent>> bytes_, const s64 limit_) :
	MemoryBaseArchive(),
	_bytes(bytes_),
	_limit(limit_) {}

s64 MemoryReadonlyArchive::totalSize() const noexcept {
	return _limit >= 0 ? std::min(_limit, static_cast<s64>(_bytes.size())) : static_cast<s64>(_bytes.size());
}

bool MemoryReadonlyArchive::hasSizeLimit() const noexcept {
	return _limit >= 0LL;
}

s64 MemoryReadonlyArchive::sizeLimit() const noexcept {
	return _limit;
}

void MemoryReadonlyArchive::setSizeLimit(const s64 limit_) {
	_limit = limit_;
}

void MemoryReadonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eStore) [[unlikely]]
	{
		return;
	}

	if (const auto total = static_cast<u64>(totalSize()); _pos >= 0 && size_ <= total && static_cast<u64>(_pos) <= total - size_) {

		std::memcpy(value_, &_bytes[_pos], size_);
		_pos += static_cast<s64>(size_);

	} else [[unlikely]]
	{
		setError();
	}
}
