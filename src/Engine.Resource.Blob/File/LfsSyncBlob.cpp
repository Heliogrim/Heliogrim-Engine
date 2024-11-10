#include "LfsSyncBlob.hpp"

using namespace hg::engine::resource;
using namespace hg;

LfsSyncBlob::LfsSyncBlob(mref<fs::Path> path_) noexcept :
	InheritMeta(),
	_path(path_),
	_stream(std::in_place) {
	if (not std::filesystem::exists(path_)) {
		std::filesystem::create_directories(path_.parentPath());
		_stream->open(static_cast<cref<std::filesystem::path>>(path_), std::ios::out);
		_stream->close();
	}
	_stream->open(static_cast<cref<std::filesystem::path>>(path_), std::ios::in | std::ios::out | std::ios::binary);
	::hg::assertrt(_stream->is_open());
}

std::span<_::byte> LfsSyncBlob::read(streamoff offset_, mref<std::span<_::byte>> dst_) const {

	::hg::assertrt(not _stream.empty() && _stream->is_open() && _stream->good());

	auto& stream = const_cast<std::fstream&>(_stream.get());
	stream.seekg(0, std::ios::end);
	const auto filesize = static_cast<streamsize>(stream.tellg());

	::hg::assertrt(offset_ >= 0uLL && offset_ <= filesize);

	const auto beg = offset_;
	const auto length = (std::min)(filesize - offset_, static_cast<streamsize>(dst_.size()));

	auto subspan = std::span { dst_ };
	stream.seekg(beg, std::ios::beg);

	while (true) {
		stream.read(std::bit_cast<char*>(subspan.data()), length);
		break;
	}

	::hg::assertd(stream.tellg() == (beg + length));
	return std::move(dst_).subspan(0uLL, length);
}

std::span<_::byte> LfsSyncBlob::write(streamoff offset_, mref<std::span<_::byte>> src_) {

	::hg::assertrt(not _stream.empty() && _stream->is_open() && _stream->good());

	auto& stream = _stream.get();
	stream.seekg(0, std::ios::end);
	const auto filesize = static_cast<streamsize>(stream.tellg());

	// Note: Guarantee that the offset is rewriting or directly appending without empty blocks in between.
	::hg::assertrt(offset_ >= 0uLL && offset_ <= filesize);

	const auto beg = offset_;
	const auto length = static_cast<streamsize>(src_.size());

	auto subspan = std::span { src_ };
	stream.seekg(beg, std::ios::beg);

	while (true) {
		stream.write(std::bit_cast<char*>(subspan.data()), subspan.size());
		break;
	}

	::hg::assertd(stream.tellg() == (beg + length));
	return std::move(src_);
}

bool LfsSyncBlob::trim(size_t tailSize_) {

	::hg::assertrt(not _stream.empty() && _stream->is_open() && _stream->good());

	if (tailSize_ <= 0) {
		return false;
	}

	/**/

	_stream->flush();

	_stream->seekg(0, std::ios::end);
	const auto filesize = static_cast<streamsize>(_stream->tellg());
	_stream->close();

	/**/

	const auto nextSize = filesize - (tailSize_ < filesize ? tailSize_ : filesize);
	std::filesystem::resize_file(_path, nextSize);

	/**/

	_stream->open(_path, std::ios::in | std::ios::out | std::ios::binary);
	::hg::assertrt(_stream->is_open());

	return true;
}

streamsize LfsSyncBlob::size() const noexcept {
	::hg::assertrt(not _stream.empty() && _stream->is_open() && _stream->good());
	auto& stream = const_cast<std::fstream&>(_stream.get());
	stream.seekg(0, std::ios::end);
	return stream.tellg();
}
