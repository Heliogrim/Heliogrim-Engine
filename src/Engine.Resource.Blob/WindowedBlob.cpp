#include "WindowedBlob.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <Engine.Asserts/Asserts.hpp>

using namespace hg::engine::resource;
using namespace hg;

WindowedBlob::WindowedBlob(ref<const Blob> source_, const streamoff base_, const streamsize size_, const size_t window_) :
	InheritMeta(),
	_source(source_),
	_base(base_),
	_size(size_),
	_window(window_),
	_loaded(-1),
	_loadedLen(0) {

	::hg::assertrt(base_ >= 0LL && size_ >= 0LL && window_ > 0uz);
	// load() reads at _base + start, which must not overflow.
	::hg::assertrt(size_ <= std::numeric_limits<streamoff>::max() - base_);
}

void WindowedBlob::load(const size_t index_) const {

	const auto start = static_cast<streamoff>(index_ * _window);
	const auto len = (std::min)(static_cast<streamsize>(_window), _size - start);

	_buffer.resize(static_cast<size_t>(len));
	auto got = _source.read(_base + start, std::span { _buffer.data(), _buffer.size() });

	_loaded = start;
	_loadedLen = got.size();
}

std::span<_::byte> WindowedBlob::read(streamoff offset_, mref<std::span<_::byte>> dst_) const {

	::hg::assertrt(offset_ >= 0LL && offset_ <= _size);

	const auto total = (std::min)(static_cast<size_t>(_size - offset_), dst_.size());
	size_t done = 0uz;

	while (done < total) {
		const auto at = static_cast<size_t>(offset_) + done;
		const auto start = static_cast<streamoff>(at / _window * _window);

		if (_loaded != start) {
			load(at / _window);
		}

		const auto inner = at - static_cast<size_t>(start);
		if (inner >= _loadedLen) {
			break;
		}

		const auto step = (std::min)(total - done, _loadedLen - inner);
		std::memcpy(dst_.data() + done, _buffer.data() + inner, step);
		done += step;
	}

	return std::move(dst_).subspan(0uz, done);
}

std::span<_::byte> WindowedBlob::write(streamoff, mref<std::span<_::byte>>) {
	::hg::assertrt(false);
	return {};
}

bool WindowedBlob::trim(size_t) {
	::hg::assertrt(false);
	return false;
}

streamsize WindowedBlob::size() const noexcept {
	return _size;
}
