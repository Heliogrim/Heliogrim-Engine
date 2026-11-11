#include "pch.h"

/**/

#include "Scratch.hpp"

/**/

#include <algorithm>
#include <atomic>
#include <charconv>
#include <format>
#include <string>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <cerrno>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/**/

namespace StorageModule {
	namespace {
		[[nodiscard]] unsigned long currentPid() {
			#if defined(_WIN32)
			return ::GetCurrentProcessId();
			#else
			return static_cast<unsigned long>(::getpid());
			#endif
		}

		[[nodiscard]] bool isAlive(const unsigned long pid_) {
			#if defined(_WIN32)
			const auto handle = ::OpenProcess(SYNCHRONIZE, FALSE, pid_);
			if (handle == nullptr) {
				// Note: Access denied means the process exists but is not ours to open.
				return ::GetLastError() == ERROR_ACCESS_DENIED;
			}
			const auto alive = ::WaitForSingleObject(handle, 0) == WAIT_TIMEOUT;
			::CloseHandle(handle);
			return alive;
			#else
			return ::kill(static_cast<pid_t>(pid_), 0) == 0 || errno == EPERM;
			#endif
		}

		void removeTree(const std::filesystem::path& path_) {
			auto ec = std::error_code {};
			// Note: Tests make files read-only; give write access back so the removal cannot stop at one.
			for (const auto& entry : std::filesystem::recursive_directory_iterator { path_, ec }) {
				std::filesystem::permissions(entry.path(), std::filesystem::perms::owner_write, std::filesystem::perm_options::add, ec);
			}
			std::filesystem::remove_all(path_, ec);
		}

		class ScratchEnvironment :
			public testing::Environment {
		public:
			void SetUp() override {
				sweepDeadScratch();
			}

			void TearDown() override {
				removeTree(scratchRoot());
				sweepDeadScratch();
			}
		};

		[[maybe_unused]] const auto* const registered = testing::AddGlobalTestEnvironment(new ScratchEnvironment {});
	}

	const std::filesystem::path& scratchRoot() {
		static const auto root = [] {
			auto path = std::filesystem::temp_directory_path() / "heliogrim-test" / "storage" / std::to_string(currentPid());
			std::filesystem::create_directories(path);
			return path;
		}();
		return root;
	}

	void sweepDeadScratch() {
		auto ec = std::error_code {};
		for (const auto& entry : std::filesystem::directory_iterator { scratchRoot().parent_path(), ec }) {
			const auto name = entry.path().filename().string();
			auto pid = 0uL;
			const auto [end, error] = std::from_chars(name.data(), name.data() + name.size(), pid);
			// Note: Leave anything that is not a process root alone.
			if (error != std::errc {} || end != name.data() + name.size()) {
				continue;
			}
			if (pid == currentPid() || isAlive(pid)) {
				continue;
			}
			removeTree(entry.path());
		}
	}

	ScratchDir::ScratchDir() {
		static auto counter = std::atomic_size_t { 0uz };

		const auto* const info = testing::UnitTest::GetInstance()->current_test_info();
		auto name = info != nullptr ? std::format("{}.{}", info->test_suite_name(), info->name()) : std::string { "global" };
		std::ranges::replace(name, '/', '_');

		path = scratchRoot() / std::format("{}-{}", name, counter.fetch_add(1uz));
		std::filesystem::create_directories(path);
	}

	ScratchDir::~ScratchDir() {
		removeTree(path);
	}
}
