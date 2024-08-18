#pragma once

#include <algorithm>
#include <ranges>
#include <span>
#include <utility>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Types.hpp>

#include "../Blob.hpp"

namespace hg::engine::resource {
	class ByteSpanBlob final :
		public Blob {
	public:
		constexpr ByteSpanBlob() noexcept = default;

		constexpr explicit ByteSpanBlob(std::span<_::byte> bytes_) noexcept :
			Blob(),
			_bytes(bytes_) {}

		constexpr ByteSpanBlob(cref<ByteSpanBlob>) noexcept = default;

		constexpr ByteSpanBlob(mref<ByteSpanBlob>) noexcept = default;

		constexpr ~ByteSpanBlob() override = default;

	private:
		std::span<_::byte> _bytes;

	public:
		constexpr std::span<_::byte> read(
			streamoff offset_,
			mref<std::span<_::byte>> dst_
		) const override {
			::hg::assertrt(offset_ >= 0LL && offset_ <= _bytes.size());
			auto copyrange = _bytes.subspan(
				offset_,
				std::min(dst_.size(), static_cast<std::size_t>(_bytes.size() - offset_))
			);
			std::ranges::copy(copyrange, dst_.data());
			return std::move(dst_).subspan(0, copyrange.size());
		}

		constexpr std::span<_::byte> write(
			streamoff offset_,
			mref<std::span<_::byte>> src_
		) override {
			::hg::assertrt(offset_ >= 0LL && offset_ <= _bytes.size() && offset_ + src_.size() <= _bytes.size());
			std::ranges::copy(src_, _bytes.begin() + offset_);
			return std::move(src_);
		}
	};
}
