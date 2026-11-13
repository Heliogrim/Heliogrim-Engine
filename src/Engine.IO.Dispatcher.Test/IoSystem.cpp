#include "pch.h"

/**/

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>
#include <Engine.Common/stdafx.h>
#include <Engine.IO.Dispatcher/IoSystem.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg;
using namespace hg::engine::io;
using namespace hg::engine::async;

namespace IoModule {
	namespace {
		constexpr size_t kFileSize = 4uz * 1024uz * 1024uz;

		[[nodiscard]] bool waitReady(auto& fut_) {
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds { 10 };
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

			[[nodiscard]] std::filesystem::path makeFile(const char* name_) const {
				const auto path = dir / name_;
				std::vector<char> data(kFileSize);
				for (size_t i = 0; i < data.size(); ++i) {
					data[i] = static_cast<char>(i % 251);
				}
				std::ofstream out { path, std::ios::binary };
				out.write(data.data(), static_cast<std::streamsize>(data.size()));
				return path;
			}

			[[nodiscard]] static Arci<IoFile> open(IoSystem& sys_, const std::filesystem::path& p_, IoOpenMode m_) {
				auto fut = sys_.open(p_, m_);
				EXPECT_TRUE(waitReady(fut));
				auto res = await(std::move(fut));
				EXPECT_TRUE(res.has_value());
				return res.has_value() ? std::move(res.value()) : Arci<IoFile> {};
			}
		};

		struct Holder :
			IoKeepAlive {
			Vector<_::byte> data;
		};
	}

	TEST_F(IoSystemTest, ReadBatchReturnsBytes) {
		IoSystem sys {};
		sys.start();
		const auto file = open(sys, makeFile("a.bin"), IoOpenMode::eRead);
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
		const auto file = open(sys, makeFile("b.bin"), IoOpenMode::eReadWrite);
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
		const auto file = open(sys, makeFile("c.bin"), IoOpenMode::eRead);
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
		const auto file = open(sys, makeFile("d.bin"), IoOpenMode::eRead);
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
		const auto file = open(sys, makeFile("e.bin"), IoOpenMode::eRead);
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
		const auto file = open(sys, makeFile("f.bin"), IoOpenMode::eRead);
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
		holder.reset();
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
		sys.stop();
	}
}
