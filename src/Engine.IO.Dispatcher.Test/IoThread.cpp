#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <thread>
#include <Engine.Common/stdafx.h>
#include <Engine.IO.Dispatcher/IoThread.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg;
using namespace hg::engine::io;
using namespace hg::engine::async;

namespace IoModule {
	namespace {
		[[nodiscard]] bool waitUntil(auto&& pred_) {
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds { 5 };
			while (not pred_()) {
				if (std::chrono::steady_clock::now() >= deadline) {
					return false;
				}
				std::this_thread::yield();
			}
			return true;
		}

		// stop() on a helper thread so a hang aborts instead of stalling the suite.
		void stopBounded(IoThread& io_) {
			std::promise<void> done {};
			auto fut = done.get_future();
			std::thread worker {
				[&] {
					io_.stop();
					done.set_value();
				}
			};
			if (fut.wait_for(std::chrono::seconds { 5 }) != std::future_status::ready) {
				std::fputs("IoThread::stop watchdog\n", stderr);
				std::abort();
			}
			worker.join();
		}

		struct LogScope {
			LogScope() { Logger::setup(); }

			~LogScope() { Logger::destroy(); }
		};
	}

	TEST(IoThread, PostWakesAndResolves) {
		const LogScope log {};
		IoThread io {};
		io.start();

		auto* const self = &io.thread();
		auto value = io.thread().post([self] { return AffineThread::current() == self ? 7 : -1; });
		ASSERT_TRUE(waitUntil([&] { return value.isReady(); }));
		auto result = await(std::move(value));
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 7);

		io.stop();
	}

	TEST(IoThread, StopIsClean) {
		const LogScope log {};
		IoThread io {};
		io.start();

		std::atomic_int counter { 0 };
		for (auto i = 0; i < 1000; ++i) {
			io.thread().postDetached([&counter] { ++counter; });
		}

		stopBounded(io);
		EXPECT_EQ(counter.load(), 1000);
		EXPECT_NE(io.port(), nullptr);

		// Rejected without running or touching the port.
		io.thread().postDetached([&counter] { ++counter; });
		EXPECT_EQ(counter.load(), 1000);
	}

	TEST(IoThread, DoubleStopIsSafe) {
		const LogScope log {};
		IoThread io {};
		io.start();
		stopBounded(io);
		stopBounded(io);
		EXPECT_NE(io.port(), nullptr);
	}

	#ifdef _DEBUG
	TEST(IoThreadDeathTest, ForeignKeyAsserts) {
		EXPECT_DEATH(
			{
				IoThread io {};
				io.start();
				::PostQueuedCompletionStatus(static_cast<HANDLE>(io.port()), 0, 1, nullptr);
				std::this_thread::sleep_for(std::chrono::seconds { 2 });
				io.stop();
			},
			""
		);
	}
	#endif
}
