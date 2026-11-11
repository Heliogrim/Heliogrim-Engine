#include "pch.h"

/**/

#include "Scratch.hpp"

/**/

#include <filesystem>
#include <fstream>
#include <string>

#if !defined(_WIN32)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

/**/

namespace StorageModule {
	namespace {
		[[nodiscard]] std::string deadPid() {
			#if defined(_WIN32)
			// Note: Windows process ids are multiples of 4, so `3` never names a process.
			return "3";
			#else
			const auto child = ::fork();
			if (child == 0) {
				::_exit(0);
			}
			::waitpid(child, nullptr, 0);
			return std::to_string(child);
			#endif
		}

		[[nodiscard]] std::string alivePid() {
			#if defined(_WIN32)
			// Note: `4` is the System process; opening it is denied, which counts as alive.
			return "4";
			#else
			// Note: `1` is init or launchd; signalling it is denied, which counts as alive.
			return "1";
			#endif
		}

		void putReadOnly(const std::filesystem::path& file_) {
			std::filesystem::create_directories(file_.parent_path());
			std::ofstream { file_, std::ios::binary } << "x";
			std::filesystem::permissions(file_, std::filesystem::perms::owner_write, std::filesystem::perm_options::remove);
		}
	}

	TEST(ScratchTest, DirIsBelowRootAndRemovedWithReadOnlyContent) {
		auto path = std::filesystem::path {};
		{
			const ScratchDir scratch {};
			path = scratch.path;
			EXPECT_EQ(path.parent_path(), scratchRoot());
			EXPECT_TRUE(std::filesystem::is_directory(path));
			putReadOnly(path / "a" / "b" / "ro.bin");
		}
		EXPECT_FALSE(std::filesystem::exists(path));
	}

	TEST(ScratchTest, SweepRemovesDeadProcessRootsOnly) {
		const auto base = scratchRoot().parent_path();
		const auto dead = base / deadPid();
		const auto alive = base / alivePid();
		ASSERT_NE(dead, alive);
		const auto foreign = base / "not-a-pid";

		putReadOnly(dead / "ro.bin");
		putReadOnly(alive / "ro.bin");
		putReadOnly(foreign / "ro.bin");
		putReadOnly(scratchRoot() / "own" / "ro.bin");

		sweepDeadScratch();

		EXPECT_FALSE(std::filesystem::exists(dead));
		EXPECT_TRUE(std::filesystem::exists(alive / "ro.bin"));
		EXPECT_TRUE(std::filesystem::exists(foreign / "ro.bin"));
		EXPECT_TRUE(std::filesystem::exists(scratchRoot() / "own" / "ro.bin"));

		for (const auto& dir : { alive, foreign, scratchRoot() / "own" }) {
			auto ec = std::error_code {};
			std::filesystem::permissions(dir / "ro.bin", std::filesystem::perms::owner_write, std::filesystem::perm_options::add, ec);
			std::filesystem::remove_all(dir, ec);
		}
	}
}
