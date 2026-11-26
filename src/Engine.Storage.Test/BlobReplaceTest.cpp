#include "pch.h"

/**/

import Engine.IO.Dispatcher;
import Engine.Resource.Blob;
import std;

using namespace hg;
using namespace hg::engine::resource;
using namespace hg::engine::io;

namespace StorageModule {
	namespace {
		[[nodiscard]] Vector<_::byte> pattern(const size_t size_, const u8 seed_) {
			Vector<_::byte> out(size_);
			for (size_t i = 0; i < size_; ++i) {
				out[i] = static_cast<_::byte>(seed_ + i);
			}
			return out;
		}
	}

	TEST(BlobReplace, SpanReplaceCopiesImage) {
		auto storage = pattern(64uz, 1u);
		ByteSpanBlob blob { std::span<_::byte> { storage }, 16uz };

		auto image = pattern(40uz, 100u);
		auto begun = blob.beginReplace(40);
		ASSERT_TRUE(begun.has_value());
		auto replacement = std::move(begun).value();
		replacement->write(0, std::span<_::byte> { image });

		ASSERT_TRUE(blob.finishReplace(std::move(replacement)).has_value());
		EXPECT_EQ(blob.size(), 40);
		EXPECT_TRUE(std::equal(image.begin(), image.end(), storage.begin()));
	}

	TEST(BlobReplace, SpanReplaceTooLargeIsDiskFull) {
		auto storage = pattern(64uz, 1u);
		const auto before = storage;
		ByteSpanBlob blob { std::span<_::byte> { storage }, 16uz };

		auto image = pattern(80uz, 100u);
		auto begun = blob.beginReplace(80);
		ASSERT_TRUE(begun.has_value());
		auto replacement = std::move(begun).value();
		replacement->write(0, std::span<_::byte> { image });

		auto done = blob.finishReplace(std::move(replacement));
		ASSERT_FALSE(done.has_value());
		EXPECT_EQ(done.error().kind, CommitError::Kind::eDiskFull);
		EXPECT_EQ(done.error().bytesNeeded, 80uLL);
		EXPECT_EQ(blob.size(), 16);
		EXPECT_EQ(storage, before);
	}

	TEST(BlobReplace, DroppedReplacementLeavesSpan) {
		auto storage = pattern(64uz, 1u);
		const auto before = storage;
		ByteSpanBlob blob { std::span<_::byte> { storage }, 16uz };

		{
			auto image = pattern(40uz, 100u);
			auto begun = blob.beginReplace(40);
			ASSERT_TRUE(begun.has_value());
			auto replacement = std::move(begun).value();
			replacement->write(0, std::span<_::byte> { image });
		}

		EXPECT_EQ(blob.size(), 16);
		EXPECT_EQ(storage, before);
	}

	TEST(BlobReplace, WindowedBlobUnsupported) {
		auto storage = pattern(64uz, 1u);
		ByteSpanBlob source { std::span<_::byte> { storage } };
		WindowedBlob windowed { source, 0, 64, 16uz };

		auto begun = windowed.beginReplace(8);
		ASSERT_FALSE(begun.has_value());
		EXPECT_EQ(begun.error().kind, CommitError::Kind::eUnsupportedVolume);
	}
}
