#include "pch.h"

/**/

#include <array>
#include <filesystem>
#include <fstream>
#include <Engine.Common/stdafx.h>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Resource.Blob/File/IoFileBlob.hpp>

/**/

using namespace hg;
using namespace hg::engine;

namespace IoModule {
	TEST(IoFileBlobTest, ReadWriteTrim) {
		Logger::setup();
		const auto path = std::filesystem::temp_directory_path() / "heliogrim-test" / "io-blob" /
			(std::to_string(::GetCurrentProcessId()) + ".bin");
		std::filesystem::create_directories(path.parent_path());
		{ std::ofstream { path, std::ios::binary }; }

		{
			io::IoSystem sys {};
			sys.start();

			auto opened = await(sys.open(path, io::IoOpenMode::eReadWrite));
			ASSERT_TRUE(opened.has_value());
			resource::IoFileBlob blob { sys, std::move(opened.value()), sys.createQueue(io::IoPriority::eNormal) };

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
}
