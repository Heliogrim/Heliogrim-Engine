#include "IoFileBlob.hpp"

#include <Engine.Common/__macro.hpp>

#include "ReplaceSequence.hpp"

import Engine.Asserts;
import Engine.Common;
import Engine.Reflect;
import Engine.Logging;
import std;

using namespace hg::engine::resource;
using namespace hg::engine;
using namespace hg;

namespace {
	// Future errors are a variant; anything but an IoError (a broken future) has no OS code.
	[[nodiscard]] u32 osErrorOf(const auto& err_) noexcept {
		const auto* io = std::get_if<io::IoError>(&err_);
		return io != nullptr ? static_cast<u32>(io->osError) : io::error_io_device;
	}
}

/**/

IoFileBlob::IoFileBlob(ref<io::IoSystem> system_, Arci<io::IoFile> file_, Arci<io::IoQueue> queue_, std::filesystem::path path_) :
	InheritMeta(),
	_system(system_),
	_file(std::move(file_)),
	_queue(std::move(queue_)),
	_size(0),
	_path(std::move(path_)) {

	// A failed size request leaves -1; the opener maps it to an access error.
	auto size = await(_system.size(_file));
	_size = size.has_value() ? static_cast<streamsize>(size.value()) : -1LL;
}

IoFileBlob::~IoFileBlob() {
	_file.reset();
	if (_temp && not _finished) {
		std::error_code ec {};
		std::filesystem::remove(_path, ec);
	}
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

	if (_file == nullptr) {
		// A spent blob (after a replace) has no file.
		_lastError = io::error_invalid_handle;
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
			_lastError = completion.osError;
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

	if (_file == nullptr) {
		_lastError = io::error_invalid_handle;
		return dst_.subspan(0uz, 0uz);
	}

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
	if (_file != nullptr) {
		_size = (std::max)(_size, offset_ + static_cast<streamoff>(written.size()));
	}
	return written;
}

bool IoFileBlob::trim(const size_t tailSize_) {

	if (tailSize_ <= 0uz) {
		return false;
	}

	if (_file == nullptr) {
		_lastError = io::error_invalid_handle;
		return false;
	}

	const auto cut = (std::min)(static_cast<streamsize>(tailSize_), _size);
	const auto next = _size - cut;

	auto res = await(_system.resize(_file, static_cast<u64>(next)));
	if (not res.has_value()) {
		IM_CORE_ERROR("Io blob failed to resize its file.");
		_lastError = osErrorOf(res.error());
		return false;
	}

	_size = next;
	return true;
}

Result<Rc<Blob>, io::CommitError> IoFileBlob::beginReplace(const streamsize size_) {

	if (_file == nullptr) {
		_lastError = io::error_invalid_handle;
		return Result<Rc<Blob>, io::CommitError>::unexpected(
			io::CommitError { .path = pathText(_path), .osError = io::error_invalid_handle }
		);
	}

	if (auto full = checkReplaceSpace(_path, size_); full.has_value()) {
		return Result<Rc<Blob>, io::CommitError>::unexpected(std::move(full).value());
	}

	io::sweepStaleTemps(_path);

	auto temp = await(_system.openTemp(_path));
	if (not temp.has_value()) {
		_lastError = osErrorOf(temp.error());
		return Result<Rc<Blob>, io::CommitError>::unexpected(
			io::CommitError {
				.kind = io::commitKindOf(osErrorOf(temp.error())), .path = pathText(_path), .osError = osErrorOf(temp.error())
			}
		);
	}

	auto repl = Rc<IoFileBlob>::create(
		_system,
		std::move(temp).value().file,
		_system.createQueue(io::IoPriority::eNormal),
		std::move(temp).value().path
	);
	repl->_temp = true;
	return Result<Rc<Blob>, io::CommitError>::expected(std::move(repl));
}

Result<void, io::CommitError> IoFileBlob::finishReplace(mref<Rc<Blob>> replacement_) {

	const auto repl = replacement_ != nullptr ? Cast<IoFileBlob>(replacement_.operator->()) : nullptr;
	if (repl == nullptr || not repl->_temp || repl->_finished) {
		return Result<void, io::CommitError>::unexpected(io::CommitError { .path = pathText(_path) });
	}

	const auto flushed = [this](ref<const Arci<io::IoFile>> file_) {
		auto res = await(_system.flush(file_));
		return res.has_value() ? 0u : osErrorOf(res.error());
	};

	const ReplaceSteps steps {
		.flushTemp = [&]() { return flushed(repl->_file); },
		.renameTemp = [&]() {
			auto res = await(_system.renameOver(repl->_file, _path));
			return res.has_value() ? 0u : osErrorOf(res.error());
		},
		.replaceByName = [&]() {
			// MoveFileExW fails while any handle on either file is open; the blob is spent after the move.
			repl->_file.reset();
			_file.reset();
			_size = -1LL;
			auto res = await(_system.replaceByName(repl->_path, _path));
			return res.has_value() ? 0u : osErrorOf(res.error());
		},
		.flushTarget = [&](const bool byName_) {
			return byName_ ? flushByName(_path) : flushed(repl->_file);
		}
	};

	auto result = runReplaceSteps(_path, steps, repl->_finished);
	// The old handle points at the unlinked file; the blob is spent and callers reopen by path.
	if (repl->_finished) {
		_file.reset();
		_size = -1LL;
	}
	if (not result.has_value()) {
		_lastError = result.error().osError;
	}
	return result;
}

streamsize IoFileBlob::size() const noexcept {
	return _size;
}

u32 IoFileBlob::lastError() const noexcept {
	return _lastError.load();
}
