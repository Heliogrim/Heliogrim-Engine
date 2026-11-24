#include "IoFileBlob.hpp"

#include <Engine.Common/__macro.hpp>

import Engine.Asserts;
import Engine.Common;
import Engine.Logging;
import std;

using namespace hg::engine::resource;
using namespace hg::engine;
using namespace hg;

IoFileBlob::IoFileBlob(ref<io::IoSystem> system_, Arci<io::IoFile> file_, Arci<io::IoQueue> queue_) :
	InheritMeta(),
	_system(system_),
	_file(std::move(file_)),
	_queue(std::move(queue_)),
	_size(0) {

	// A failed size request leaves -1; the opener maps it to an access error.
	auto size = await(_system.size(_file));
	_size = size.has_value() ? static_cast<streamsize>(size.value()) : -1LL;
}

size_t IoFileBlob::transfer(
	const io::IoOp op_,
	const streamoff offset_,
	const std::span<_::byte> buffer_,
	const size_t piece_
) const {

	::hg::assertrt(offset_ >= 0LL);
	::hg::assertd(_system.running());

	if (not _system.running()) {
		IM_CORE_ERROR("Io blob used after its io system stopped.");
		return 0uz;
	}

	Vector<io::IoRequest> requests {};
	for (size_t at = 0uz; at < buffer_.size(); at += piece_) {
		requests.push_back(
			io::IoRequest {
				.op = op_, .file = _file, .offset = static_cast<u64>(offset_) + at,
				.buffer = buffer_.subspan(at, (std::min)(piece_, buffer_.size() - at))
			}
		);
	}
	const auto count = requests.size();
	if (count == 0uz) {
		return 0uz;
	}

	auto res = await(_queue->submit(std::move(requests)));
	if (not res.has_value() || res.value().size() != count) {
		IM_CORE_ERROR("Io blob request was not resolved.");
		return 0uz;
	}

	size_t done = 0uz;
	for (const auto& completion : res.value()) {
		if (completion.status == io::IoStatus::eFailed) {
			IM_CORE_ERRORF("Io blob request failed with os error {}.", completion.osError);
			return 0uz;
		}
		done += completion.bytes;
		if (completion.status != io::IoStatus::eDone) {
			break;
		}
	}
	return done;
}

std::span<_::byte> IoFileBlob::read(const streamoff offset_, mref<std::span<_::byte>> dst_) const {

	if (dst_.empty()) {
		return dst_.subspan(0uz, 0uz);
	}

	::hg::assertrt(offset_ >= 0LL);

	const auto align = static_cast<u64>(_file->alignment());
	const auto first = static_cast<u64>(offset_);
	const auto lo = first / align * align;
	const auto hi = (first + dst_.size() + align - 1uLL) / align * align;

	const auto direct = lo == first && hi == first + dst_.size() &&
		reinterpret_cast<std::uintptr_t>(dst_.data()) % align == 0uLL;
	if (direct) {
		return dst_.subspan(0uz, transfer(io::IoOp::eRead, offset_, dst_, max_read_request));
	}

	// shortcut: one copy out of the bounce; aligned callers (sector-aligned archive bases) skip it.
	const auto span = static_cast<size_t>(hi - lo);
	const auto storage = std::make_unique_for_overwrite<_::byte[]>(span + align);
	const auto base = reinterpret_cast<ptr<_::byte>>(
		(reinterpret_cast<std::uintptr_t>(storage.get()) + align - 1uLL) / align * align
	);

	const auto got = transfer(io::IoOp::eRead, static_cast<streamoff>(lo), { base, span }, max_read_request);
	const auto skip = static_cast<size_t>(first - lo);
	const auto len = got > skip ? (std::min)(got - skip, dst_.size()) : 0uz;
	std::memcpy(dst_.data(), base + skip, len);
	return dst_.subspan(0uz, len);
}

std::span<_::byte> IoFileBlob::write(const streamoff offset_, mref<std::span<_::byte>> src_) {

	auto written = src_.subspan(0uz, transfer(io::IoOp::eWrite, offset_, src_, src_.size()));
	_size = (std::max)(_size, offset_ + static_cast<streamoff>(written.size()));
	return written;
}

bool IoFileBlob::trim(const size_t tailSize_) {

	if (tailSize_ <= 0uz) {
		return false;
	}

	const auto cut = (std::min)(static_cast<streamsize>(tailSize_), _size);
	const auto next = _size - cut;

	auto res = await(_system.resize(_file, static_cast<u64>(next)));
	if (not res.has_value()) {
		IM_CORE_ERROR("Io blob failed to resize its file.");
		return false;
	}

	_size = next;
	return true;
}

streamsize IoFileBlob::size() const noexcept {
	return _size;
}
