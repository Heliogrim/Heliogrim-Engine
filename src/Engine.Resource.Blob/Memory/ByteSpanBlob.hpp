#pragma once

#include <algorithm>
#include <ranges>
#include <span>
#include <utility>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "../Blob.hpp"

namespace hg::engine::resource {
	class ByteSpanBlob final :
		public InheritMeta<ByteSpanBlob, Blob> {
	public:
		constexpr ByteSpanBlob() noexcept = default;

		constexpr explicit ByteSpanBlob(const std::span<_::byte> bytes_, const size_t used_) noexcept :
			InheritMeta(),
			_bytes(bytes_),
			_used(used_) {
			::hg::assertd(used_ <= bytes_.size());
		}

		constexpr explicit ByteSpanBlob(const std::span<_::byte> bytes_) noexcept :
			ByteSpanBlob(bytes_, bytes_.size()) {}

		constexpr ByteSpanBlob(cref<ByteSpanBlob>) noexcept = default;

		constexpr ByteSpanBlob(mref<ByteSpanBlob>) noexcept = default;

		constexpr ~ByteSpanBlob() override = default;

	private:
		std::span<_::byte> _bytes;
		size_t _used;

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
			_used = std::max(_used, static_cast<std::size_t>(offset_ + src_.size()));
			return std::move(src_);
		}

		bool trim(size_t tailSize_) override {
			#if _DEBUG
			const auto size = _used;
			const auto start = _used - (tailSize_ > size ? size : tailSize_);
			std::memset(_bytes.data() + start, NULL, size - start);
			#endif
			_used -= std::min(_used, tailSize_);
			return true;
		}

	public:
		[[nodiscard]] constexpr streamsize capacity() const noexcept {
			return _bytes.size();
		}

		[[nodiscard]] constexpr streamsize size() const noexcept override {
			//return static_cast<streamsize>(_bytes.size());
			return _used;
		}
	};
}
