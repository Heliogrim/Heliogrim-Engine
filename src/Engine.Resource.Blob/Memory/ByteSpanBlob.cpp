#include "ByteSpanBlob.hpp"

#include "HeapBlob.hpp"

using namespace ::hg::engine::resource;
using namespace ::hg;

namespace io = ::hg::engine::io;

Result<Rc<Blob>, io::CommitError> ByteSpanBlob::beginReplace(const streamsize size_) {
	return Result<Rc<Blob>, io::CommitError>::expected(
		Rc<HeapBlob>::create(static_cast<size_t>(std::max(size_, streamsize { 0 })))
	);
}

Result<void, io::CommitError> ByteSpanBlob::finishReplace(mref<Rc<Blob>> replacement_) {
	if (replacement_ == nullptr) {
		return Result<void, io::CommitError>::unexpected(io::CommitError {});
	}

	const auto image = replacement_->size();
	if (image < 0LL || static_cast<u64>(image) > static_cast<u64>(_bytes.size())) {
		return Result<void, io::CommitError>::unexpected(
			io::CommitError {
				.kind = io::CommitError::Kind::eDiskFull,
				.bytesNeeded = static_cast<u64>(std::max(image, streamsize { 0 }))
			}
		);
	}

	// All or nothing: a short read leaves the span untouched.
	auto scratch = Vector<_::byte>(static_cast<size_t>(image));
	const auto copied = replacement_->read(0, std::span<_::byte> { scratch });
	if (copied.size() != static_cast<size_t>(image)) {
		return Result<void, io::CommitError>::unexpected(
			io::CommitError { .kind = io::CommitError::Kind::eDamagedSource }
		);
	}
	std::ranges::copy(scratch, _bytes.begin());

	_used = static_cast<size_t>(image);
	return Result<void, io::CommitError>::expected();
}
