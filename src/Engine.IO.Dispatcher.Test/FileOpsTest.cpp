#include "pch.h"

/**/

#include <Engine.Common/stdafx.h>

import Engine.IO.Dispatcher;
import Engine.Logging;
import Engine.Scheduler;
import std;

using namespace hg;
using namespace hg::engine::io;
using namespace hg::engine::async;

namespace IoModule {
	namespace {
		class FileOps :
			public ::testing::Test {
		protected:
			std::filesystem::path dir;

			void SetUp() override {
				Logger::setup();
				const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
				dir = std::filesystem::temp_directory_path() / "heliogrim-test" / "fileops" /
					std::to_string(::GetCurrentProcessId()) / info->name();
				std::filesystem::remove_all(dir);
				std::filesystem::create_directories(dir);
			}

			void TearDown() override {
				std::error_code ec {};
				std::filesystem::remove_all(dir, ec);
				Logger::destroy();
			}

			static void put(const std::filesystem::path& p_, const std::string_view text_) {
				std::ofstream out { p_, std::ios::binary };
				out << text_;
			}

			[[nodiscard]] static std::string readHandle(const ptr<void> handle_) {
				char buf[16] {};
				DWORD read = 0;
				(void)::ReadFile(handle_, buf, sizeof(buf), &read, nullptr);
				return { buf, read };
			}

			// Shares delete: the renamed temp handle may still be open (it has DELETE access).
			[[nodiscard]] static std::string slurp(const std::filesystem::path& p_) {
				const auto handle = ::CreateFileW(
					p_.c_str(),
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					nullptr,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					nullptr
				);
				if (handle == INVALID_HANDLE_VALUE) {
					return "<open failed>";
				}
				auto text = readHandle(handle);
				::CloseHandle(handle);
				return text;
			}

			[[nodiscard]] static ptr<void> hold(const std::filesystem::path& p_, const DWORD share_) {
				return ::CreateFileW(p_.c_str(), GENERIC_READ, share_, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			}

			static void write(const ptr<void> handle_, const std::string_view text_) {
				DWORD written = 0;
				ASSERT_TRUE(::WriteFile(handle_, text_.data(), static_cast<DWORD>(text_.size()), &written, nullptr));
			}

		};

		using IoSystemOps = FileOps;

		constexpr DWORD share_all = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		constexpr DWORD share_no_delete = FILE_SHARE_READ | FILE_SHARE_WRITE;
	}

	TEST_F(FileOps, CreateTempSiblingIsUniqueHgtmp) {
		const auto target = dir / "target.bin";
		auto a = createTempSibling(target, 0u);
		auto b = createTempSibling(target, 0u);
		ASSERT_TRUE(a.has_value());
		ASSERT_TRUE(b.has_value());

		EXPECT_NE(a->path, b->path);
		EXPECT_EQ(a->path.parent_path(), dir);
		EXPECT_EQ(b->path.parent_path(), dir);
		const std::regex pattern { R"(^target\.bin\.[0-9a-f]{8}\.hgtmp$)" };
		EXPECT_TRUE(std::regex_match(a->path.filename().string(), pattern));
		EXPECT_TRUE(std::regex_match(b->path.filename().string(), pattern));
		EXPECT_TRUE(std::filesystem::exists(a->path));
		EXPECT_TRUE(std::filesystem::exists(b->path));

		::CloseHandle(a->handle);
		::CloseHandle(b->handle);
	}

	TEST_F(FileOps, RenameOverKeepsShareDeleteReaderOnOld) {
		const auto target = dir / "target.bin";
		put(target, "old");
		const auto reader = hold(target, share_all);
		ASSERT_NE(reader, INVALID_HANDLE_VALUE);

		auto temp = createTempSibling(target, 0u);
		ASSERT_TRUE(temp.has_value());
		ASSERT_NO_FATAL_FAILURE(write(temp->handle, "new"));
		EXPECT_EQ(flushFile(temp->handle), 0u);
		EXPECT_EQ(renameOver(temp->handle, target), 0u);

		EXPECT_EQ(readHandle(reader), "old");
		EXPECT_EQ(slurp(target), "new");
		EXPECT_FALSE(std::filesystem::exists(temp->path));

		::CloseHandle(reader);
		::CloseHandle(temp->handle);
	}

	TEST_F(FileOps, RenameOverBlockedByHandleWithoutShareDelete) {
		const auto target = dir / "target.bin";
		put(target, "old");
		const auto blocker = hold(target, share_no_delete);
		ASSERT_NE(blocker, INVALID_HANDLE_VALUE);

		auto temp = createTempSibling(target, 0u);
		ASSERT_TRUE(temp.has_value());
		ASSERT_NO_FATAL_FAILURE(write(temp->handle, "new"));

		const auto error = renameOver(temp->handle, target);
		EXPECT_TRUE(error == ERROR_SHARING_VIOLATION || error == ERROR_ACCESS_DENIED) << error;
		EXPECT_EQ(readHandle(blocker), "old");

		::CloseHandle(blocker);
		::CloseHandle(temp->handle);
		EXPECT_EQ(slurp(target), "old");
	}

	TEST_F(FileOps, ReplaceByNameAfterClose) {
		const auto target = dir / "target.bin";
		put(target, "old");
		auto temp = createTempSibling(target, 0u);
		ASSERT_TRUE(temp.has_value());
		ASSERT_NO_FATAL_FAILURE(write(temp->handle, "new"));
		::CloseHandle(temp->handle);

		EXPECT_EQ(replaceByName(temp->path, target), 0u);
		EXPECT_EQ(slurp(target), "new");
		EXPECT_FALSE(std::filesystem::exists(temp->path));
	}

	TEST_F(FileOps, SweepRemovesOnlyUnlockedTemps) {
		const auto target = dir / "target.bin";
		const auto stale = dir / "target.bin.0000000a.hgtmp";
		const auto locked = dir / "target.bin.0000000b.hgtmp";
		put(target, "keep");
		put(stale, "x");
		put(locked, "y");
		const auto blocker = hold(locked, share_no_delete);
		ASSERT_NE(blocker, INVALID_HANDLE_VALUE);

		sweepStaleTemps(target);

		EXPECT_FALSE(std::filesystem::exists(stale));
		EXPECT_TRUE(std::filesystem::exists(locked));
		EXPECT_TRUE(std::filesystem::exists(target));
		::CloseHandle(blocker);
	}

	TEST_F(FileOps, SweepLeavesOpenTemp) {
		const auto target = dir / "target.bin";
		put(target, "keep");
		auto temp = createTempSibling(target, 0u);
		ASSERT_TRUE(temp.has_value());

		sweepStaleTemps(target);
		EXPECT_TRUE(std::filesystem::exists(temp->path));

		::CloseHandle(temp->handle);
		sweepStaleTemps(target);
		EXPECT_FALSE(std::filesystem::exists(temp->path));
		EXPECT_TRUE(std::filesystem::exists(target));
	}

	TEST_F(IoSystemOps, ReplaceByNameOnIoThread) {
		const auto target = dir / "target.bin";
		put(target, "old");

		IoSystem sys {};
		sys.start();
		const auto waitFor = [](auto& fut_) {
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds { 10 };
			while (not fut_.isReady() && std::chrono::steady_clock::now() < deadline) {
				std::this_thread::yield();
			}
			return fut_.isReady();
		};

		auto openFut = sys.openTemp(target);
		ASSERT_TRUE(waitFor(openFut));
		auto opened = await(std::move(openFut));
		ASSERT_TRUE(opened.has_value());
		auto temp = std::move(opened.value());

		Vector<_::byte> data { static_cast<_::byte>('n'), static_cast<_::byte>('e'), static_cast<_::byte>('w') };
		const auto queue = sys.createQueue(IoPriority::eNormal);
		Vector<IoRequest> w {};
		w.push_back(IoRequest { .op = IoOp::eWrite, .file = temp.file, .offset = 0uLL, .buffer = { data.data(), data.size() } });
		auto writeFut = queue->submit(std::move(w));
		ASSERT_TRUE(waitFor(writeFut));
		ASSERT_TRUE(await(std::move(writeFut)).has_value());

		temp.file.reset();
		auto replaceFut = sys.replaceByName(temp.path, target);
		ASSERT_TRUE(waitFor(replaceFut));
		EXPECT_TRUE(await(std::move(replaceFut)).has_value());

		EXPECT_EQ(slurp(target), "new");
		EXPECT_FALSE(std::filesystem::exists(temp.path));
		sys.stop();
	}

	TEST_F(IoSystemOps, FlushAndRenameOverOnIoThread) {
		const auto target = dir / "target.bin";
		put(target, "old");
		const auto reader = hold(target, share_all);
		ASSERT_NE(reader, INVALID_HANDLE_VALUE);

		IoSystem sys {};
		sys.start();
		const auto waitFor = [](auto& fut_) {
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds { 10 };
			while (not fut_.isReady() && std::chrono::steady_clock::now() < deadline) {
				std::this_thread::yield();
			}
			return fut_.isReady();
		};

		auto openFut = sys.openTemp(target);
		ASSERT_TRUE(waitFor(openFut));
		auto opened = await(std::move(openFut));
		ASSERT_TRUE(opened.has_value());
		auto temp = std::move(opened.value());

		Vector<_::byte> data { static_cast<_::byte>('n'), static_cast<_::byte>('e'), static_cast<_::byte>('w') };
		const auto queue = sys.createQueue(IoPriority::eNormal);
		Vector<IoRequest> w {};
		w.push_back(IoRequest { .op = IoOp::eWrite, .file = temp.file, .offset = 0uLL, .buffer = { data.data(), data.size() } });
		auto writeFut = queue->submit(std::move(w));
		ASSERT_TRUE(waitFor(writeFut));
		auto written = await(std::move(writeFut));
		ASSERT_TRUE(written.has_value());
		EXPECT_EQ(written.value()[0].status, IoStatus::eDone);

		auto flushFut = sys.flush(temp.file);
		ASSERT_TRUE(waitFor(flushFut));
		EXPECT_TRUE(await(std::move(flushFut)).has_value());

		auto renameFut = sys.renameOver(temp.file, target);
		ASSERT_TRUE(waitFor(renameFut));
		EXPECT_TRUE(await(std::move(renameFut)).has_value());

		EXPECT_EQ(readHandle(reader), "old");
		EXPECT_EQ(slurp(target), "new");
		EXPECT_FALSE(std::filesystem::exists(temp.path));

		::CloseHandle(reader);
		sys.stop();
	}
}
