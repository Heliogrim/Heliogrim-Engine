#include "IoFileBlob.hpp"

#include <algorithm>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::resource;
using namespace hg::engine;
using namespace hg;

IoFileBlob::IoFileBlob(ref<io::IoSystem> system_, Arci<io::IoFile> file_, Arci<io::IoQueue> queue_) :
	InheritMeta(),
	_system(system_),
	_file(std::move(file_)),
	_queue(std::move(queue_)),
	_size(0) {

	auto size = await(_system.size(_file));
	::hg::assertrt(size.has_value());
	_size = static_cast<streamsize>(size.value());
}

std::span<_::byte> IoFileBlob::transfer(
	const io::IoOp op_,
	const streamoff offset_,
	const std::span<_::byte> buffer_
) const {

	::hg::assertrt(offset_ >= 0LL);

	Vector<io::IoRequest> requests {};
	requests.push_back(
		io::IoRequest { .op = op_, .file = _file, .offset = static_cast<u64>(offset_), .buffer = buffer_ }
	);

	auto res = await(_queue->submit(std::move(requests)));
	if (not res.has_value() || res.value().size() != 1uz) {
		IM_CORE_ERROR("Io blob request was not resolved.");
		return buffer_.subspan(0uz, 0uz);
	}

	const auto& completion = res.value().front();
	if (completion.status == io::IoStatus::eFailed) {
		IM_CORE_ERRORF("Io blob request failed with os error {}.", completion.osError);
		return buffer_.subspan(0uz, 0uz);
	}

	return buffer_.subspan(0uz, completion.bytes);
}

std::span<_::byte> IoFileBlob::read(streamoff offset_, mref<std::span<_::byte>> dst_) const {
	return transfer(io::IoOp::eRead, offset_, dst_);
}

std::span<_::byte> IoFileBlob::write(streamoff offset_, mref<std::span<_::byte>> src_) {

	auto written = transfer(io::IoOp::eWrite, offset_, src_);
	_size = (std::max)(_size, offset_ + static_cast<streamoff>(written.size()));
	return written;
}

bool IoFileBlob::trim(size_t tailSize_) {

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
