#include "pch.h"

/**/

#include <array>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Engine.Resource.Blob/WindowedBlob.hpp>
#include <Engine.Resource.Blob/Memory/ByteSpanBlob.hpp>

/**/

using namespace hg::engine::resource;
using namespace hg;

namespace StorageModule {
	namespace {
		class CountingBlob final :
			public InheritMeta<CountingBlob, Blob> {
		public:
			explicit CountingBlob(std::span<_::byte> bytes_) :
				inner(bytes_) {}

			ByteSpanBlob inner;
			mutable size_t sourceReads = 0uz;

			std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override {
				++sourceReads;
				return inner.read(offset_, std::move(dst_));
			}

			std::span<_::byte> write(streamoff, mref<std::span<_::byte>>) override { return {}; }

			bool trim(size_t) override { return false; }

			[[nodiscard]] streamsize size() const noexcept override { return inner.size(); }
		};

		Vector<_::byte> makeBytes(const size_t size_) {
			Vector<_::byte> bytes(size_);
			for (size_t i = 0; i < size_; ++i) {
				bytes[i] = static_cast<_::byte>(i % 251uz);
			}
			return bytes;
		}
	}

	TEST(WindowedBlobTest, SmallRangeIsOnePark) {
		auto bytes = makeBytes(2uz * 1024uz * 1024uz);
		CountingBlob source { std::span { bytes } };
		const WindowedBlob blob { source, 4096, 1024 * 1024 };

		for (size_t i = 0; i < 10'000uz; ++i) {
			const auto off = static_cast<streamoff>((i * 104729uz) % (1024uz * 1024uz - 7uz));
			std::array<_::byte, 7> dst {};
			auto got = blob.read(off, std::span { dst });
			ASSERT_EQ(got.size(), 7uz);
			for (size_t k = 0; k < 7uz; ++k) {
				ASSERT_EQ(got[k], bytes[4096uz + off + k]);
			}
		}
		EXPECT_EQ(source.sourceReads, 1uz);
	}

	TEST(WindowedBlobTest, LargeRangeStreamsWindows) {
		auto bytes = makeBytes(21uz * 1024uz * 1024uz);
		CountingBlob source { std::span { bytes } };
		const WindowedBlob blob { source, 1024, 20 * 1024 * 1024 };

		std::array<_::byte, 4096> dst {};
		for (size_t off = 0uz; off < 20uz * 1024uz * 1024uz; off += dst.size()) {
			auto got = blob.read(static_cast<streamoff>(off), std::span { dst });
			ASSERT_EQ(got.size(), dst.size());
			ASSERT_EQ(got[0], bytes[1024uz + off]);
		}
		EXPECT_EQ(source.sourceReads, 3uz);
	}

	TEST(WindowedBlobTest, ReadAcrossWindowEdge) {
		auto bytes = makeBytes(11uz * 1024uz * 1024uz);
		CountingBlob source { std::span { bytes } };
		const WindowedBlob blob { source, 0, 10 * 1024 * 1024 };

		std::array<_::byte, 16> dst {};
		const auto off = static_cast<streamoff>(WindowedBlob::kArchiveWindow) - 8;
		auto got = blob.read(off, std::span { dst });
		ASSERT_EQ(got.size(), 16uz);
		for (size_t k = 0; k < 16uz; ++k) {
			EXPECT_EQ(got[k], bytes[static_cast<size_t>(off) + k]);
		}
	}
}
