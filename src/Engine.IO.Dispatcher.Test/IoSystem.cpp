#include "pch.h"

/**/

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <thread>
#include <Engine.Common/stdafx.h>
#include <Engine.IO.Dispatcher/IoSystem.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

using namespace hg;
using namespace hg::engine::io;
using namespace hg::engine::async;

namespace IoModule {
	namespace {
		constexpr size_t kFileSize = 4uz * 1024uz * 1024uz;

		[[nodiscard]] bool waitReady(auto& fut_, const std::chrono::seconds timeout_ = std::chrono::seconds { 10 }) {
			const auto deadline = std::chrono::steady_clock::now() + timeout_;
			while (not fut_.isReady()) {
				if (std::chrono::steady_clock::now() >= deadline) {
					return false;
				}
				std::this_thread::yield();
			}
			return true;
		}

		class IoSystemTest :
			public ::testing::Test {
		protected:
			std::filesystem::path dir;

			void SetUp() override {
				Logger::setup();
				dir = std::filesystem::temp_directory_path() / "heliogrim-test" / "io" /
					std::to_string(::GetCurrentProcessId());
				std::filesystem::create_directories(dir);
			}

			void TearDown() override {
				std::error_code ec {};
				std::filesystem::remove_all(dir, ec);
				Logger::destroy();
			}

			[[nodiscard]] std::filesystem::path makeFile(const char* name_, const size_t size_ = kFileSize) const {
				const auto path = dir / name_;
				std::vector<char> data(std::min(size_, 64uz * 1024uz * 1024uz));
				for (size_t i = 0; i < data.size(); ++i) {
					data[i] = static_cast<char>(i % 251);
				}
				std::ofstream out { path, std::ios::binary };
				for (size_t done = 0; done < size_; done += data.size()) {
					out.write(data.data(), static_cast<std::streamsize>(std::min(data.size(), size_ - done)));
				}
				return path;
			}

			// Fatal failures only return from here; wrap calls in ASSERT_NO_FATAL_FAILURE.
			static void open(IoSystem& sys_, const std::filesystem::path& p_, IoOpenMode m_, Arci<IoFile>& out_) {
				auto fut = sys_.open(p_, m_);
				ASSERT_TRUE(waitReady(fut));
				auto res = await(std::move(fut));
				ASSERT_TRUE(res.has_value());
				out_ = std::move(res.value());
			}

			// Reads the ReadBatchReturnsBytes pattern: 16 x 4096 at k * 65537.
			static void readPattern(
				const IoBackendKind kind_,
				const std::filesystem::path& p_,
				Vector<Vector<_::byte>>& bufs_,
				Vector<IoCompletion>& out_
			) {
				IoSystem sys { kind_ };
				sys.start();
				Arci<IoFile> file {};
				ASSERT_NO_FATAL_FAILURE(open(sys, p_, IoOpenMode::eRead, file));
				const auto queue = sys.createQueue(IoPriority::eNormal);

				bufs_.assign(16, Vector<_::byte>(4096));
				Vector<IoRequest> reqs {};
				for (u32 k = 0; k < 16; ++k) {
					reqs.push_back(
						IoRequest {
							.op = IoOp::eRead, .file = file, .offset = static_cast<u64>(k) * 65537uLL,
							.buffer = { bufs_[k].data(), bufs_[k].size() }
						}
					);
				}
				auto fut = queue->submit(std::move(reqs));
				ASSERT_TRUE(waitReady(fut));
				auto res = await(std::move(fut));
				ASSERT_TRUE(res.has_value());
				out_ = std::move(res.value());
				sys.stop();
			}
		};

		std::atomic_bool holderDestroyed { false };

		struct Holder :
			IoKeepAlive {
			Vector<_::byte> data;

			~Holder() override {
				holderDestroyed = true;
			}
		};

		[[nodiscard]] double mean(const Vector<u32>& values_) {
			double sum = 0.0;
			for (const auto value : values_) {
				sum += value;
			}
			return sum / static_cast<double>(values_.size());
		}
	}

	TEST_F(IoSystemTest, ReadBatchReturnsBytes) {
		IoSystem sys {};
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("a.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<Vector<_::byte>> bufs(16, Vector<_::byte>(4096));
		Vector<IoRequest> reqs {};
		for (u32 k = 0; k < 16; ++k) {
			reqs.push_back(
				IoRequest {
					.op = IoOp::eRead, .file = file, .offset = static_cast<u64>(k) * 65537uLL,
					.buffer = { bufs[k].data(), bufs[k].size() }
				}
			);
		}

		auto fut = queue->submit(std::move(reqs));
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_TRUE(res.has_value());
		ASSERT_EQ(res.value().size(), 16uz);
		for (u32 k = 0; k < 16; ++k) {
			EXPECT_EQ(res.value()[k].status, IoStatus::eDone);
			EXPECT_EQ(res.value()[k].bytes, 4096u);
			for (u32 i = 0; i < 4096; ++i) {
				ASSERT_EQ(static_cast<u8>(bufs[k][i]), static_cast<u8>((k * 65537uLL + i) % 251uLL));
			}
		}
		sys.stop();
	}

	TEST_F(IoSystemTest, WriteThenReadRoundTrips) {
		IoSystem sys {};
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("b.bin"), IoOpenMode::eReadWrite, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<_::byte> out(8192, static_cast<_::byte>(0xAB));
		Vector<_::byte> in(8192);
		{
			Vector<IoRequest> w {};
			w.push_back(
				IoRequest {
					.op = IoOp::eWrite, .file = file, .offset = 1uLL << 20, .buffer = { out.data(), out.size() }
				}
			);
			auto fut = queue->submit(std::move(w));
			ASSERT_TRUE(waitReady(fut));
			auto res = await(std::move(fut));
			ASSERT_TRUE(res.has_value());
			EXPECT_EQ(res.value()[0].status, IoStatus::eDone);
		}
		{
			Vector<IoRequest> r {};
			r.push_back(
				IoRequest {
					.op = IoOp::eRead, .file = file, .offset = 1uLL << 20, .buffer = { in.data(), in.size() }
				}
			);
			auto fut = queue->submit(std::move(r));
			ASSERT_TRUE(waitReady(fut));
			auto res = await(std::move(fut));
			ASSERT_TRUE(res.has_value());
			EXPECT_EQ(res.value()[0].status, IoStatus::eDone);
		}
		EXPECT_EQ(in, out);
		sys.stop();
	}

	TEST_F(IoSystemTest, ShortReadAtEndOfFile) {
		IoSystem sys {};
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("c.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<_::byte> buf(8192);
		Vector<IoRequest> reqs {};
		reqs.push_back(
			IoRequest {
				.op = IoOp::eRead, .file = file, .offset = kFileSize - 1000, .buffer = { buf.data(), buf.size() }
			}
		);
		auto fut = queue->submit(std::move(reqs));
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_TRUE(res.has_value());
		EXPECT_EQ(res.value()[0].status, IoStatus::eShort);
		EXPECT_EQ(res.value()[0].bytes, 1000u);
		sys.stop();
	}

	TEST_F(IoSystemTest, BatchesResolveFifoPerQueue) {
		IoSystem sys {};
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("d.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<Vector<_::byte>> bigBufs(32, Vector<_::byte>(65536));
		Vector<_::byte> b(4096), c(4096);

		Vector<IoRequest> a {};
		for (u32 k = 0; k < 32; ++k) {
			a.push_back(
				IoRequest {
					.op = IoOp::eRead, .file = file, .offset = static_cast<u64>(k) * 65536uLL,
					.buffer = { bigBufs[k].data(), bigBufs[k].size() }
				}
			);
		}
		Vector<IoRequest> rb {}, rc {};
		rb.push_back(IoRequest { .op = IoOp::eRead, .file = file, .offset = 0, .buffer = { b.data(), b.size() } });
		rc.push_back(IoRequest { .op = IoOp::eRead, .file = file, .offset = 0, .buffer = { c.data(), c.size() } });

		auto fa = queue->submit(std::move(a));
		auto fb = queue->submit(std::move(rb));
		auto fc = queue->submit(std::move(rc));

		// C is last in FIFO order, so A and B must already be resolved.
		ASSERT_TRUE(waitReady(fc));
		EXPECT_TRUE(fa.isReady());
		EXPECT_TRUE(fb.isReady());
		EXPECT_TRUE(await(std::move(fc)).has_value());
		sys.stop();
	}

	TEST_F(IoSystemTest, OpenMissingFileFails) {
		IoSystem sys {};
		sys.start();
		auto fut = sys.open(dir / "missing.bin", IoOpenMode::eRead);
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().as<IoError>().osError, static_cast<u32>(ERROR_FILE_NOT_FOUND));
		sys.stop();
	}

	TEST_F(IoSystemTest, CompressionOtherThanNoneFails) {
		IoSystem sys {};
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("e.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<_::byte> buf(4096);
		Vector<IoRequest> reqs {};
		reqs.push_back(
			IoRequest {
				.op = IoOp::eRead, .file = file, .offset = 0, .buffer = { buf.data(), buf.size() },
				.compression = static_cast<IoCompression>(1)
			}
		);
		auto fut = queue->submit(std::move(reqs));
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_TRUE(res.has_value());
		EXPECT_EQ(res.value()[0].status, IoStatus::eFailed);
		sys.stop();
	}

	TEST_F(IoSystemTest, DroppedFutureStillCompletes) {
		IoSystem sys {};
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("f.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		auto holder = Arci<Holder>::create();
		holder->data.resize(4096);
		Vector<IoRequest> reqs {};
		reqs.push_back(
			IoRequest {
				.op = IoOp::eRead, .file = file, .offset = 0, .buffer = { holder->data.data(), holder->data.size() },
				.keepAlive = holder.into<IoKeepAlive>()
			}
		);
		holderDestroyed = false;
		holder.reset();
		// The request still holds the keep-alive.
		EXPECT_FALSE(holderDestroyed);
		{
			auto dropped = queue->submit(std::move(reqs));
		}

		// A later batch resolving implies the dropped one finished.
		Vector<_::byte> buf(16);
		Vector<IoRequest> next {};
		next.push_back(IoRequest { .op = IoOp::eRead, .file = file, .offset = 0, .buffer = { buf.data(), buf.size() } });
		auto fut = queue->submit(std::move(next));
		ASSERT_TRUE(waitReady(fut));
		EXPECT_TRUE(await(std::move(fut)).has_value());
		// FIFO: the dropped batch resolved, and released its requests, before this one.
		EXPECT_TRUE(holderDestroyed);
		sys.stop();
	}

	TEST_F(IoSystemTest, ThousandRequestsHonourPriority) {
		constexpr u32 kPerLevel = 334;
		constexpr size_t kBlock = 64uz * 1024uz;
		const auto path = makeFile("g.bin");

		// The test engine sets up its own logger.
		Logger::destroy();
		SchedulerModule::structuredRuntimeTest(
			[&](nmpt<engine::Scheduler> s_) {
				IoSystem sys {};
				sys.start();
				Arci<IoFile> file {};
				ASSERT_NO_FATAL_FAILURE(open(sys, path, IoOpenMode::eRead, file));

				const std::array levels { IoPriority::eLow, IoPriority::eNormal, IoPriority::eHigh };
				std::array<Arci<IoQueue>, 3> queues {};
				for (size_t l = 0; l < levels.size(); ++l) {
					queues[l] = sys.createQueue(levels[l]);
				}

				// Built up front, so the submit loop only posts.
				std::mt19937_64 rng { 0x5EEDuLL };
				std::uniform_int_distribution<u64> offsets { 0uLL, kFileSize - kBlock };
				constexpr u32 total = kPerLevel * 3u;
				Vector<Vector<_::byte>> bufs(total, Vector<_::byte>(kBlock));
				Vector<Vector<IoRequest>> batches(total);
				for (u32 i = 0; i < total; ++i) {
					batches[i].push_back(
						IoRequest {
							.op = IoOp::eRead, .file = file, .offset = offsets(rng), .buffer = { bufs[i].data(), kBlock }
						}
					);
				}

				std::atomic<u32> rank { 0u };
				std::atomic<u32> completed { 0u };
				Vector<u32> ranks(total, 0u);
				std::atomic_bool stopProbe { false };
				std::atomic_bool probeDone { false };
				std::atomic<u64> probeCount { 0uLL };

				s_->exec(
					engine::scheduler::task::make_task(
						[&] {
							while (not stopProbe) {
								++probeCount;
								engine::scheduler::fiber::self::yield();
							}
							probeDone = true;
						}
					)
				);
				s_->exec(
					engine::scheduler::task::make_task(
						[&] {
							// Interleaved low, normal, high, so only priority can order the completions.
							for (u32 i = 0; i < total; ++i) {
								[[maybe_unused]] auto next = queues[i % 3u]->submit(std::move(batches[i])).then(
									[&, i](Vector<IoCompletion>&&) {
										ranks[i] = rank.fetch_add(1u);
										completed.fetch_add(1u, std::memory_order_release);
									}
								);
							}
						}
					)
				);

				const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds { 30 };
				while (completed.load(std::memory_order_acquire) < total && std::chrono::steady_clock::now() < deadline) {
					std::this_thread::yield();
				}
				stopProbe = true;
				while (not probeDone && std::chrono::steady_clock::now() < deadline) {
					std::this_thread::yield();
				}
				sys.stop();
				ASSERT_EQ(completed.load(), total);
				ASSERT_TRUE(probeDone);

				std::array<Vector<u32>, 3> perLevel {};
				for (u32 i = 0; i < total; ++i) {
					perLevel[i % 3u].push_back(ranks[i]);
				}
				EXPECT_LT(mean(perLevel[2]), mean(perLevel[1]));
				EXPECT_LT(mean(perLevel[1]), mean(perLevel[0]));
				EXPECT_GT(probeCount.load(), 0uLL);
			}
		);
		Logger::setup();
	}

	TEST_F(IoSystemTest, CancelledBufferUntouchedAfterResolve) {
		constexpr size_t kBlock = 1024uz * 1024uz;
		IoSystem sys { IoBackendKind::eIocp, 4u };
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("h.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<Vector<_::byte>> bufs(64, Vector<_::byte>(kBlock, static_cast<_::byte>(0xCD)));
		Vector<IoRequest> reqs {};
		for (u32 k = 0; k < 64; ++k) {
			reqs.push_back(
				IoRequest {
					.op = IoOp::eRead, .file = file, .offset = (k % 4u) * kBlock, .buffer = { bufs[k].data(), kBlock },
					.tag = 0x100uLL | k
				}
			);
		}
		auto fut = queue->submit(std::move(reqs));
		queue->cancel(0xF00uLL, 0x100uLL);
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_TRUE(res.has_value());

		Vector<u32> cancelled {};
		Vector<Vector<_::byte>> snapshots {};
		for (u32 k = 0; k < 64; ++k) {
			if (res.value()[k].status == IoStatus::eCancelled) {
				cancelled.push_back(k);
				snapshots.push_back(bufs[k]);
			}
		}
		ASSERT_FALSE(cancelled.empty());

		std::this_thread::sleep_for(std::chrono::milliseconds { 50 });
		for (size_t c = 0; c < cancelled.size(); ++c) {
			const auto& buf = bufs[cancelled[c]];
			EXPECT_EQ(buf, snapshots[c]);
			EXPECT_TRUE(std::ranges::all_of(buf, [](const _::byte b_) { return b_ == static_cast<_::byte>(0xCD); }));
		}
		sys.stop();
	}

	TEST_F(IoSystemTest, StopDrainsInFlight) {
		constexpr size_t kBlock = 1024uz * 1024uz;
		IoSystem sys {};
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("i.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<Vector<_::byte>> bufs(64, Vector<_::byte>(kBlock));
		Vector<Future<Vector<IoCompletion>>> futs {};
		for (u32 k = 0; k < 64; ++k) {
			Vector<IoRequest> reqs {};
			reqs.push_back(
				IoRequest {
					.op = IoOp::eRead, .file = file, .offset = (k % 4u) * kBlock, .buffer = { bufs[k].data(), kBlock }
				}
			);
			futs.push_back(queue->submit(std::move(reqs)));
		}
		sys.stop();

		for (auto& fut : futs) {
			ASSERT_TRUE(fut.isReady());
			auto res = await(std::move(fut));
			ASSERT_TRUE(res.has_value());
			const auto status = res.value()[0].status;
			EXPECT_TRUE(status == IoStatus::eDone || status == IoStatus::eCancelled);
		}
	}

	TEST_F(IoSystemTest, BlockingBackendMatchesIocp) {
		const auto path = makeFile("j.bin");
		Vector<Vector<_::byte>> iocpBufs {}, blockingBufs {};
		Vector<IoCompletion> iocpRes {}, blockingRes {};
		ASSERT_NO_FATAL_FAILURE(readPattern(IoBackendKind::eIocp, path, iocpBufs, iocpRes));
		ASSERT_NO_FATAL_FAILURE(readPattern(IoBackendKind::eBlocking, path, blockingBufs, blockingRes));

		EXPECT_EQ(iocpBufs, blockingBufs);
		ASSERT_EQ(iocpRes.size(), blockingRes.size());
		for (size_t k = 0; k < iocpRes.size(); ++k) {
			EXPECT_EQ(iocpRes[k].status, IoStatus::eDone);
			EXPECT_EQ(iocpRes[k].status, blockingRes[k].status);
			EXPECT_EQ(iocpRes[k].bytes, blockingRes[k].bytes);
		}
	}

	class IoSystemBench :
		public IoSystemTest {};

	// Random reads over a freshly written (so mostly cached) file; one batch per run.
	TEST_F(IoSystemBench, DISABLED_Throughput) {
		constexpr size_t kFile = 1024uz * 1024uz * 1024uz;
		const auto path = makeFile("bench.bin", kFile);

		const auto run = [&](const IoBackendKind kind_, const u32 maxInFlight_, const size_t block_, const size_t total_) {
			IoSystem sys { kind_, maxInFlight_ };
			sys.start();
			Arci<IoFile> file {};
			open(sys, path, IoOpenMode::eRead, file);
			const auto queue = sys.createQueue(IoPriority::eNormal);

			// shortcut: requests share a pool of 256 buffers and contents are not checked; fine for throughput only.
			Vector<Vector<_::byte>> pool(256, Vector<_::byte>(block_));
			std::mt19937_64 rng { 0xBE7CuLL };
			std::uniform_int_distribution<u64> blocks { 0uLL, kFile / block_ - 1uLL };
			const auto count = total_ / block_;
			Vector<IoRequest> reqs {};
			for (size_t i = 0; i < count; ++i) {
				reqs.push_back(
					IoRequest {
						.op = IoOp::eRead, .file = file, .offset = blocks(rng) * block_,
						.buffer = { pool[i % pool.size()].data(), block_ }
					}
				);
			}

			const auto start = std::chrono::steady_clock::now();
			auto fut = queue->submit(std::move(reqs));
			EXPECT_TRUE(waitReady(fut, std::chrono::seconds { 120 }));
			const auto seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
			sys.stop();
			return static_cast<double>(total_) / (1024.0 * 1024.0) / seconds;
		};

		constexpr size_t kTotal = 256uz * 1024uz * 1024uz;
		for (const size_t block : { 64uz * 1024uz, 1024uz * 1024uz }) {
			const auto iocp = run(IoBackendKind::eIocp, IoSystem::kMaxInFlight, block, kTotal);
			const auto blocking = run(IoBackendKind::eBlocking, IoSystem::kMaxInFlight, block, kTotal);
			std::printf("backend block=%zuKiB iocp=%.0fMiB/s blocking=%.0fMiB/s\n", block / 1024uz, iocp, blocking);
		}
		for (const size_t block : { 4uz * 1024uz, 1024uz * 1024uz }) {
			for (const u32 inFlight : { 8u, 16u, 32u, 64u, 128u }) {
				const auto iocp = run(IoBackendKind::eIocp, inFlight, block, block == 4096uz ? kTotal / 4uz : kTotal);
				std::printf("sweep block=%zuKiB maxInFlight=%u iocp=%.0fMiB/s\n", block / 1024uz, inFlight, iocp);
			}
		}
	}
}
