#include "pch.h"

/**/

#include <Engine.Common/stdafx.h>

import Engine.Logging;
import Engine.Resource.Blob;
import std;

/**/

using namespace hg;
using namespace hg::engine;

namespace IoModule {
	TEST(IoFileBlobTest, ReadWriteTrim) {
		Logger::setup();
		const auto path = std::filesystem::temp_directory_path() / "heliogrim-test" / "io-blob" /
			(std::to_string(::GetCurrentProcessId()) + ".bin");
		std::filesystem::create_directories(path.parent_path());

		/**/

		{
			std::ofstream { path, std::ios::binary };
		}

		/**/

		{
			io::IoSystem sys {};
			sys.start();

			auto opened = await(sys.open(path, io::IoOpenMode::eReadWrite));
			ASSERT_TRUE(opened.has_value());
			resource::IoFileBlob blob { sys, std::move(opened.value()), sys.createQueue(io::IoPriority::eNormal), path };

			std::array<_::byte, 100> src {};
			for (size_t i = 0; i < src.size(); ++i) {
				src[i] = static_cast<_::byte>(i);
			}
			EXPECT_EQ(blob.write(0, std::span { src }).size(), 100uz);
			EXPECT_EQ(blob.size(), 100);

			std::array<_::byte, 100> dst {};
			auto got = blob.read(0, std::span { dst });
			ASSERT_EQ(got.size(), 100uz);
			EXPECT_EQ(dst, src);

			EXPECT_TRUE(blob.trim(40));
			EXPECT_EQ(blob.size(), 60);
		}

		std::error_code ec {};
		std::filesystem::remove(path, ec);
		Logger::destroy();
	}

	// Unbuffered reads at any offset, length and buffer address return the same bytes as the file holds.
	TEST(IoFileBlobTest, UnbufferedReadsAnyRange) {
		Logger::setup();
		const auto path = std::filesystem::temp_directory_path() / "heliogrim-test" / "io-blob" /
			(std::to_string(::GetCurrentProcessId()) + "-unbuffered.bin");
		std::filesystem::create_directories(path.parent_path());

		constexpr size_t fileSize = 1024uz * 1024uz + 1000uz;
		Vector<_::byte> content(fileSize);
		for (size_t i = 0; i < content.size(); ++i) {
			content[i] = static_cast<_::byte>((i * 7uz) % 253uz);
		}
		{
			std::ofstream out { path, std::ios::binary };
			out.write(reinterpret_cast<ptr<const char>>(content.data()), static_cast<std::streamsize>(content.size()));
		}

		for (const auto kind : { io::IoBackendKind::eIocp, io::IoBackendKind::eBlocking }) {
			SCOPED_TRACE(kind == io::IoBackendKind::eIocp ? "iocp" : "blocking");
			io::IoSystem sys { kind };
			sys.start();

			{
				const auto rejected = await(sys.open(path, static_cast<io::IoOpenMode>(0x81u)));
				ASSERT_FALSE(rejected.has_value()) << "unbuffered read-write must be rejected";
				EXPECT_EQ(rejected.error().as<io::IoError>().osError, static_cast<u32>(ERROR_NOT_SUPPORTED));
			}

			auto opened = await(sys.open(path, io::IoOpenMode::eUnbuffered));
			ASSERT_TRUE(opened.has_value());
			EXPECT_EQ(opened.value()->alignment(), 4096u);
			resource::IoFileBlob blob { sys, std::move(opened.value()), sys.createQueue(io::IoPriority::eNormal), path };
			EXPECT_EQ(blob.size(), static_cast<streamsize>(fileSize));

			// Room for an aligned window inside, so one case takes the direct path.
			Vector<_::byte> storage(fileSize + 8192uz);
			const auto aligned = reinterpret_cast<ptr<_::byte>>(
				(reinterpret_cast<std::uintptr_t>(storage.data()) + 4095uz) / 4096uz * 4096uz
			);

			struct Case {
				size_t offset;
				size_t length;
				size_t expect;
				bool direct;
			};
			for (const auto& c : {
				Case { .offset = 12345uz, .length = 700000uz, .expect = 700000uz, .direct = false },
				Case { .offset = 0uz, .length = 3uz, .expect = 3uz, .direct = false },
				Case { .offset = fileSize - 100uz, .length = 4096uz, .expect = 100uz, .direct = false },
				Case { .offset = 8192uz, .length = 512uz * 1024uz, .expect = 512uz * 1024uz, .direct = true },
				Case { .offset = 1024uz * 1024uz, .length = 8192uz, .expect = 1000uz, .direct = true },
				Case { .offset = fileSize - 300000uz, .length = 600000uz, .expect = 300000uz, .direct = false }
			}) {
				const auto dst = c.direct ? std::span { aligned, c.length } : std::span { storage.data() + 1uz, c.length };
				std::fill(dst.begin(), dst.end(), _::byte { 0xEE });
				const auto got = blob.read(static_cast<streamoff>(c.offset), std::span { dst });
				ASSERT_EQ(got.size(), c.expect) << "offset " << c.offset;
				EXPECT_TRUE(std::equal(got.begin(), got.end(), content.begin() + static_cast<std::ptrdiff_t>(c.offset)))
					<< "offset " << c.offset;
			}

			EXPECT_EQ(blob.read(0, std::span<_::byte> {}).size(), 0uz);

			sys.stop();
		}

		std::error_code ec {};
		std::filesystem::remove(path, ec);
		Logger::destroy();
	}
}
