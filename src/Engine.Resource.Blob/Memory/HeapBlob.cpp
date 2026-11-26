#include "HeapBlob.hpp"

using namespace ::hg::engine::resource;
using namespace ::hg;

HeapBlob::HeapBlob(const size_t reserve_) :
	InheritMeta() {
	_bytes.reserve(reserve_);
}

std::span<_::byte> HeapBlob::read(const streamoff offset_, mref<std::span<_::byte>> dst_) const {
	if (offset_ < 0LL || static_cast<u64>(offset_) > _bytes.size()) {
		return dst_.first(0uz);
	}

	const auto count = std::min(dst_.size(), _bytes.size() - static_cast<size_t>(offset_));
	std::copy_n(_bytes.begin() + offset_, count, dst_.begin());
	return dst_.first(count);
}

std::span<_::byte> HeapBlob::write(const streamoff offset_, mref<std::span<_::byte>> src_) {
	if (offset_ < 0LL || static_cast<u64>(offset_) > _bytes.size()) {
		return src_.first(0uz);
	}

	const auto start = static_cast<size_t>(offset_);
	if (src_.size() > std::numeric_limits<size_t>::max() - start) {
		return src_.first(0uz);
	}

	const auto end = start + src_.size();
	if (end > _bytes.size()) {
		_bytes.resize(end);
	}
	std::ranges::copy(src_, _bytes.begin() + offset_);
	return src_;
}

bool HeapBlob::trim(const size_t tailSize_) {
	_bytes.resize(_bytes.size() - std::min(_bytes.size(), tailSize_));
	return true;
}

std::span<const _::byte> HeapBlob::bytes() const noexcept {
	return _bytes;
}

streamsize HeapBlob::size() const noexcept {
	return static_cast<streamsize>(_bytes.size());
}
