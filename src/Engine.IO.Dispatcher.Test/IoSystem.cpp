#include "pch.h"

/**/

#include <Engine.Common/stdafx.h>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

import Engine.IO.Dispatcher;
import Engine.Logging;
import Engine.Scheduler;
import std;

using namespace hg;
using namespace hg::engine::io;
using namespace hg::engine::async;

namespace IoModule {
	namespace {
		inline constexpr static size_t default_file_size = 4uz * 1024uz * 1024uz;

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

			[[nodiscard]] std::filesystem::path makeFile(const char* name_, const size_t size_ = default_file_size) const {
				const auto path = dir / name_;
				Vector<char> data(std::min(size_, 64uz * 1024uz * 1024uz));
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

		// Incompressible data written unbuffered, so the drive and the OS cache add no tricks; kept if it exists.
		[[nodiscard]] std::filesystem::path benchFile(ref<const std::filesystem::path> dir_, const u64 bytes_) {
			const auto* const env = std::getenv("HG_IO_BENCH_FILE");
			auto path = env != nullptr ? std::filesystem::path { env } : dir_ / "bench-unbuffered.bin";
			if (std::filesystem::exists(path) && std::filesystem::file_size(path) >= bytes_) {
				return path;
			}

			const auto handle = ::CreateFileW(
				path.c_str(),
				GENERIC_WRITE,
				0,
				nullptr,
				CREATE_ALWAYS,
				FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
				nullptr
			);
			if (handle == INVALID_HANDLE_VALUE) {
				return {};
			}
			constexpr size_t block = 1024uz * 1024uz;
			auto* const data = static_cast<ptr<u32>>(::VirtualAlloc(nullptr, block, MEM_COMMIT, PAGE_READWRITE));
			if (data == nullptr) {
				::CloseHandle(handle);
				return {};
			}
			std::mt19937 rng { 1u };
			for (u64 at = 0; at < bytes_; at += block) {
				for (size_t i = 0; i < block / sizeof(u32); ++i) {
					data[i] = rng();
				}
				DWORD written = 0;
				if (not ::WriteFile(handle, data, static_cast<DWORD>(block), &written, nullptr) || written != block) {
					::VirtualFree(data, 0, MEM_RELEASE);
					::CloseHandle(handle);
					return {};
				}
			}
			::VirtualFree(data, 0, MEM_RELEASE);
			::CloseHandle(handle);
			return path;
		}

		// Page-aligned buffers for unbuffered reads, released with the pool.
		class AlignedPool {
		public:
			AlignedPool(const size_t count_, const size_t block_) :
				base(static_cast<ptr<_::byte>>(::VirtualAlloc(nullptr, count_ * block_, MEM_COMMIT, PAGE_READWRITE))),
				block(block_) {}

			AlignedPool(ref<const AlignedPool>) = delete;

			AlignedPool(mref<AlignedPool>) = delete;

			~AlignedPool() {
				::VirtualFree(base, 0, MEM_RELEASE);
			}

			ref<AlignedPool> operator=(ref<const AlignedPool>) = delete;

			ref<AlignedPool> operator=(mref<AlignedPool>) = delete;

		public:
			ptr<_::byte> base;
			size_t block;

		public:
			[[nodiscard]] std::span<_::byte> at(const size_t index_) const noexcept {
				return { base + index_ * block, block };
			}
		};

		[[nodiscard]] double percentile(Vector<double>& values_, const double p_) {
			if (values_.empty()) {
				return 0.0;
			}
			std::ranges::sort(values_);
			return values_[static_cast<size_t>(p_ * static_cast<double>(values_.size() - 1uz))];
		}

		constexpr u64 bench_file_size = 4uLL * 1024uLL * 1024uLL * 1024uLL;
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
		Vector<_::byte> in(8192); {
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
		} {
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
				.op = IoOp::eRead, .file = file, .offset = default_file_size - 1000, .buffer = { buf.data(), buf.size() }
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

	// Each request alone exceeds the byte budget; it must still issue once nothing else is in flight.
	TEST_F(IoSystemTest, RequestsOverByteBudgetComplete) {
		IoSystem sys { IoBackendKind::eIocp, IoLimits { .maxInFlight = 32u, .maxInFlightBytes = 4096u } };
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("budget.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<Vector<_::byte>> bufs(8, Vector<_::byte>(65536));
		Vector<IoRequest> reqs {};
		for (u32 k = 0; k < 8; ++k) {
			reqs.push_back(
				IoRequest {
					.op = IoOp::eRead, .file = file, .offset = static_cast<u64>(k) * 65536uLL,
					.buffer = { bufs[k].data(), bufs[k].size() }
				}
			);
		}
		auto fut = queue->submit(std::move(reqs));
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_TRUE(res.has_value());
		for (const auto& completion : res.value()) {
			EXPECT_EQ(completion.status, IoStatus::eDone);
			EXPECT_EQ(completion.bytes, 65536u);
		}
		EXPECT_EQ(sys.requestsIssued(), 8uLL);
		sys.stop();
	}

	// Limits set before start() are the ones the thread runs with: one in flight, four requests, all complete.
	TEST_F(IoSystemTest, SetLimitsBeforeStartTakesEffect) {
		IoSystem sys {};
		sys.setLimits(IoLimits { .maxInFlight = 1u, .maxInFlightBytes = 4096u });
		sys.start();
		Arci<IoFile> file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("limits.bin"), IoOpenMode::eRead, file));
		const auto queue = sys.createQueue(IoPriority::eNormal);

		Vector<Vector<_::byte>> bufs(4, Vector<_::byte>(4096));
		Vector<IoRequest> reqs {};
		for (u32 k = 0; k < 4; ++k) {
			reqs.push_back(
				IoRequest {
					.op = IoOp::eRead, .file = file, .offset = static_cast<u64>(k) * 4096uLL,
					.buffer = { bufs[k].data(), bufs[k].size() }
				}
			);
		}
		auto fut = queue->submit(std::move(reqs));
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_TRUE(res.has_value());
		ASSERT_EQ(res.value().size(), 4uz);
		for (const auto& completion : res.value()) {
			EXPECT_EQ(completion.status, IoStatus::eDone);
			EXPECT_EQ(completion.bytes, 4096u);
		}
		EXPECT_EQ(sys.requestsIssued(), 4uLL);
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

	TEST_F(IoSystemTest, OpenReadWriteCreatesMissingDirectories) {
		IoSystem sys {};
		sys.start();
		const auto path = dir / "new" / "nested" / "created.bin";
		auto fut = sys.open(path, IoOpenMode::eReadWrite);
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		EXPECT_TRUE(res.has_value());
		EXPECT_TRUE(std::filesystem::exists(path));
		sys.stop();
	}

	TEST_F(IoSystemTest, OpenReadIntoMissingDirectoryCreatesNothing) {
		IoSystem sys {};
		sys.start();
		const auto missing = dir / "absent";
		auto fut = sys.open(missing / "file.bin", IoOpenMode::eRead);
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().as<IoError>().osError, static_cast<u32>(ERROR_PATH_NOT_FOUND));
		EXPECT_FALSE(std::filesystem::exists(missing));
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
		EXPECT_FALSE(holderDestroyed); {
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
				std::uniform_int_distribution<u64> offsets { 0uLL, default_file_size - kBlock };
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
		IoSystem sys { IoBackendKind::eIocp, IoLimits { .maxInFlight = 4u } };
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

		// Contract: eCancelled means zero bytes were written; a partly filled abort reports eShort.
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

	TEST_F(IoSystemTest, CancelIssuedRequest) {
		// A pipe with no writer: the read stays pending until CancelIoEx aborts it.
		const auto name = std::format(LR"(\\.\pipe\heliogrim-io-test-{})", ::GetCurrentProcessId());
		const auto server = ::CreateNamedPipeW(
			name.c_str(),
			PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_BYTE,
			1,
			0,
			0,
			0,
			nullptr
		);
		ASSERT_NE(server, INVALID_HANDLE_VALUE);

		IoSystem sys {};
		sys.start();
		Arci<IoFile> pipe {}, file {};
		ASSERT_NO_FATAL_FAILURE(open(sys, name, IoOpenMode::eRead, pipe));
		ASSERT_NO_FATAL_FAILURE(open(sys, makeFile("k.bin"), IoOpenMode::eRead, file));

		// Strict priority: the low marker issues only after the high pipe read, so its resolve proves the issue.
		const auto high = sys.createQueue(IoPriority::eHigh);
		const auto low = sys.createQueue(IoPriority::eLow);
		Vector<_::byte> pipeBuf(4096, static_cast<_::byte>(0xCD)), markBuf(4096);
		Vector<IoRequest> pipeReq {}, markReq {};
		pipeReq.push_back(
			IoRequest { .op = IoOp::eRead, .file = pipe, .offset = 0uLL, .buffer = { pipeBuf.data(), pipeBuf.size() }, .tag = 0x7uLL }
		);
		markReq.push_back(IoRequest { .op = IoOp::eRead, .file = file, .offset = 0uLL, .buffer = { markBuf.data(), markBuf.size() } });

		auto fut = high->submit(std::move(pipeReq));
		auto mark = low->submit(std::move(markReq));
		ASSERT_TRUE(waitReady(mark));
		EXPECT_FALSE(fut.isReady());

		high->cancel(~0uLL, 0x7uLL);
		ASSERT_TRUE(waitReady(fut));
		auto res = await(std::move(fut));
		ASSERT_TRUE(res.has_value());
		EXPECT_EQ(res.value()[0].status, IoStatus::eCancelled);
		EXPECT_EQ(res.value()[0].bytes, 0u);
		EXPECT_TRUE(std::ranges::all_of(pipeBuf, [](const _::byte b_) { return b_ == static_cast<_::byte>(0xCD); }));

		sys.stop();
		::CloseHandle(server);
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
			IoSystem sys { kind_, IoLimits { .maxInFlight = maxInFlight_, .maxInFlightBytes = ~0u } };
			sys.start();
			Arci<IoFile> file {};
			EXPECT_NO_FATAL_FAILURE(open(sys, path, IoOpenMode::eRead, file));
			if (HasFatalFailure()) {
				sys.stop();
				return 0.0;
			}
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
			const auto iocp = run(IoBackendKind::eIocp, 64u, block, kTotal);
			const auto blocking = run(IoBackendKind::eBlocking, 64u, block, kTotal);
			std::printf("backend block=%zuKiB iocp=%.0fMiB/s blocking=%.0fMiB/s\n", block / 1024uz, iocp, blocking);
		}
		for (const size_t block : { 4uz * 1024uz, 1024uz * 1024uz }) {
			for (const u32 inFlight : { 8u, 16u, 32u, 64u, 128u }) {
				const auto iocp = run(IoBackendKind::eIocp, inFlight, block, block == 4096uz ? kTotal / 4uz : kTotal);
				std::printf("sweep block=%zuKiB maxInFlight=%u iocp=%.0fMiB/s\n", block / 1024uz, inFlight, iocp);
			}
		}
	}

	// W03.6 done-when: unbuffered random reads through the io system against the W03.5 drive spike (rio.csv).
	TEST_F(IoSystemBench, DISABLED_UnbufferedThroughput) {
		const auto path = benchFile(dir, bench_file_size);
		ASSERT_FALSE(path.empty());

		const auto run = [&](const size_t block_, const IoLimits limits_) {
			IoSystem sys { IoBackendKind::eIocp, limits_ };
			sys.start();
			Arci<IoFile> file {};
			EXPECT_NO_FATAL_FAILURE(open(sys, path, IoOpenMode::eUnbuffered, file));
			if (HasFatalFailure()) {
				sys.stop();
				return 0.0;
			}
			const auto queue = sys.createQueue(IoPriority::eNormal);

			constexpr size_t total = 2048uz * 1024uz * 1024uz;
			const auto count = total / block_;
			AlignedPool pool { 256uz, block_ };
			std::mt19937_64 rng { 0xBE7CuLL };
			std::uniform_int_distribution<u64> blocks { 0uLL, bench_file_size / block_ - 1uLL };
			Vector<IoRequest> reqs {};
			for (size_t i = 0; i < count; ++i) {
				reqs.push_back(
					IoRequest { .op = IoOp::eRead, .file = file, .offset = blocks(rng) * block_, .buffer = pool.at(i % 256uz) }
				);
			}

			const auto start = std::chrono::steady_clock::now();
			auto fut = queue->submit(std::move(reqs));
			EXPECT_TRUE(waitReady(fut, std::chrono::seconds { 120 }));
			const auto seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
			sys.stop();
			return static_cast<double>(total) / (1024.0 * 1024.0) / seconds;
		};

		std::printf("block_kib,max_in_flight,budget_kib,mib_s\n");
		for (const size_t block : { 64uz * 1024uz, 256uz * 1024uz }) {
			for (const auto limits : {
				IoLimits { .maxInFlight = 8u, .maxInFlightBytes = ~0u },
				IoLimits { .maxInFlight = 32u, .maxInFlightBytes = ~0u },
				IoLimits { .maxInFlight = 32u, .maxInFlightBytes = 1024u * 1024u },
				IoLimits { .maxInFlight = 32u, .maxInFlightBytes = 2048u * 1024u }
			}) {
				const auto mib = run(block, limits);
				std::printf(
					"%zu,%u,%u,%.0f\n",
					block / 1024uz,
					limits.maxInFlight,
					limits.maxInFlightBytes == ~0u ? 0u : limits.maxInFlightBytes / 1024u,
					mib
				);
			}
		}
	}

	// W03.6 burst check (Q03.9): bulk low-priority 256 KiB reads with high-priority 64 KiB reads arriving during
	// the burst, per in-flight byte budget. Prints the burst makespan and the high-priority latency.
	TEST_F(IoSystemBench, DISABLED_Burst) {
		const auto path = benchFile(dir, bench_file_size);
		ASSERT_FALSE(path.empty());

		constexpr size_t bulkBlock = 256uz * 1024uz;
		constexpr size_t bulkCount = 8192uz;
		constexpr size_t hotBlock = 64uz * 1024uz;
		constexpr auto hotGap = std::chrono::microseconds { 500 };

		const auto run = [&](const u32 budget_, const bool burst_) {
			IoSystem sys { IoBackendKind::eIocp, IoLimits { .maxInFlight = 32u, .maxInFlightBytes = budget_ } };
			sys.start();
			Arci<IoFile> file {};
			EXPECT_NO_FATAL_FAILURE(open(sys, path, IoOpenMode::eUnbuffered, file));
			if (HasFatalFailure()) {
				sys.stop();
				return;
			}
			const auto bulk = sys.createQueue(IoPriority::eLow);
			const auto hot = sys.createQueue(IoPriority::eHigh);

			AlignedPool bulkPool { 256uz, bulkBlock };
			AlignedPool hotPool { 1uz, hotBlock };
			std::mt19937_64 rng { 0xB0257uLL };
			std::uniform_int_distribution<u64> bulkAt { 0uLL, bench_file_size / bulkBlock - 1uLL };
			std::uniform_int_distribution<u64> hotAt { 0uLL, bench_file_size / hotBlock - 1uLL };

			Vector<IoRequest> reqs {};
			for (size_t i = 0; i < (burst_ ? bulkCount : 1uz); ++i) {
				reqs.push_back(
					IoRequest {
						.op = IoOp::eRead, .file = file, .offset = bulkAt(rng) * bulkBlock, .buffer = bulkPool.at(i % 256uz)
					}
				);
			}

			const auto start = std::chrono::steady_clock::now();
			auto bulkDone = bulk->submit(std::move(reqs));
			auto makespan = 0.0;
			Vector<double> latency {};

			// Without a burst: a fixed number of hot reads as the idle reference.
			while (burst_ ? not bulkDone.isReady() : latency.size() < 2000uz) {
				const auto gapEnd = std::chrono::steady_clock::now() + hotGap;
				while (std::chrono::steady_clock::now() < gapEnd) {}

				Vector<IoRequest> one {};
				one.push_back(
					IoRequest { .op = IoOp::eRead, .file = file, .offset = hotAt(rng) * hotBlock, .buffer = hotPool.at(0uz) }
				);
				const auto t0 = std::chrono::steady_clock::now();
				auto hotDone = hot->submit(std::move(one));
				while (not hotDone.isReady()) {}
				const auto t1 = std::chrono::steady_clock::now();
				latency.push_back(std::chrono::duration<double, std::micro>(t1 - t0).count());
				if (burst_ && makespan == 0.0 && bulkDone.isReady()) {
					makespan = std::chrono::duration<double, std::milli>(t1 - start).count();
				}
			}
			if (burst_ && makespan == 0.0) {
				makespan = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
			}
			EXPECT_TRUE(waitReady(bulkDone, std::chrono::seconds { 120 }));
			sys.stop();

			const auto mib = burst_ ? static_cast<double>(bulkCount * bulkBlock) / (1024.0 * 1024.0) / (makespan / 1000.0) : 0.0;
			const auto count = latency.size();
			const auto p50 = percentile(latency, 0.50);
			const auto p99 = percentile(latency, 0.99);
			std::printf(
				"%s,%u,%.1f,%.0f,%zu,%.0f,%.0f,%.0f\n",
				burst_ ? "burst" : "idle",
				budget_ / 1024u,
				makespan,
				mib,
				count,
				p50,
				p99,
				latency.back()
			);
		};

		std::printf("mode,budget_kib,makespan_ms,bulk_mib_s,hot_n,hot_p50_us,hot_p99_us,hot_max_us\n");
		run(2048u * 1024u, false);
		for (u32 rep = 0; rep < 3u; ++rep) {
			for (const u32 budget : { 1u, 2u, 4u, 8u }) {
				run(budget * 1024u * 1024u, true);
			}
		}
	}
}
