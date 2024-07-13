#include "LfsSyncBlob.hpp"

using namespace hg::engine::resource;
using namespace hg;

LfsSyncBlob::LfsSyncBlob(mref<fs::Path> path_) noexcept :
	Blob(),
	_stream(std::in_place) {
	if (not std::filesystem::exists(path_.stdFsPath())) {
		std::filesystem::create_directories(path_.stdFsPath().parent_path());
		_stream->open(path_.stdFsPath(), std::ios::out);
		_stream->close();
	}
	_stream->open(path_.stdFsPath(), std::ios::in | std::ios::out | std::ios::binary);
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

streamsize LfsSyncBlob::size() const noexcept {
	::hg::assertrt(not _stream.empty() && _stream->is_open() && _stream->good());
	auto& stream = const_cast<std::fstream&>(_stream.get());
	stream.seekg(0, std::ios::end);
	return stream.tellg();
}
