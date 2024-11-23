#include "MemoryReadWriteArchive.hpp"

#include <cstring>
#include <utility>

#include <Engine.Common/Move.hpp>

using namespace hg::engine::resource;
using namespace hg;

MemoryReadWriteArchive::MemoryReadWriteArchive(
	mref<MemoryBaseArchive> base_,
	ref<AutoArray<_::byte>> bytes_,
	s64 limit_
) noexcept :
	MemoryBaseArchive(::hg::move(base_)),
	_bytes(bytes_),
	_limit(limit_) {}

MemoryReadWriteArchive::MemoryReadWriteArchive(ref<AutoArray<_::byte>> bytes_) :
	MemoryBaseArchive(),
	_bytes(bytes_),
	_limit(-1) {}

MemoryReadWriteArchive::MemoryReadWriteArchive(ref<AutoArray<_::byte>> bytes_, const s64 limit_) :
	MemoryBaseArchive(),
	_bytes(bytes_),
	_limit(limit_) {}

ref<MemoryReadWriteArchive::this_type> MemoryReadWriteArchive::operator=(this_type&& other_) noexcept {
	if (std::addressof(other_) != this) {
		MemoryBaseArchive::operator=(::hg::move(other_));
		_limit = other_._limit;
	}
	return *this;
}

s64 MemoryReadWriteArchive::totalSize() const noexcept {
	return _limit >= 0LL ? std::min(_limit, static_cast<s64>(_bytes.size())) : static_cast<s64>(_bytes.size());
}

bool MemoryReadWriteArchive::hasSizeLimit() const noexcept {
	return _limit >= 0;
}

s64 MemoryReadWriteArchive::sizeLimit() const noexcept {
	return _limit;
}

void MemoryReadWriteArchive::setSizeLimit(const s64 limit_) {
	_limit = limit_;
}

void MemoryReadWriteArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError()) [[unlikely]] {
		return;
	}

	if (mode_ == ArchiveStreamMode::eStore) {

		const s64 overflow { _pos + static_cast<s64>(size_) - static_cast<s64>(_bytes.size()) };
		if (overflow > 0) {

			// TODO: Add possibility to expand AutoArray<T> with uninitialized memory sequence, cause we sometimes might just override the new memory
			const auto nextSize { _bytes.size() + overflow };
			_bytes.resize(nextSize);
		}

		std::memcpy(&_bytes[_pos], value_, size_);
		_pos += static_cast<s64>(size_);

	} else /* if (mode_ == ArchiveStreamMode::eOut) */ {

		if (_pos + size_ <= _bytes.size()) {

			std::memcpy(value_, &_bytes[_pos], size_);
			_pos += static_cast<s64>(size_);

		} else [[unlikely]]
		{
			setError();
		}

	}
}
