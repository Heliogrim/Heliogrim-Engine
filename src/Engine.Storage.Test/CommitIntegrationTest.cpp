#include "pch.h"

/**/

#include "Scratch.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <Engine.Common/__macro.hpp>

import Engine.Common;
import Engine.Config;
import Engine.Diagnostics;
import Engine.IO.Dispatcher;
import Engine.Resource.Archive;
import Engine.Resource.Blob;
import Engine.Resource.Package;
import Engine.Storage.Action;
import Engine.Storage.Boot;
import Engine.Storage.Package;
import Engine.Storage.Registry;
import Engine.Storage.System;

/**/

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg::engine::resource;
using namespace hg::engine;
using namespace hg;

namespace StorageModule {
	/** Param: true runs the storage system over the IO thread, false runs it synchronously. */
	class CommitIntegration :
		public testing::TestWithParam<bool> {
	public:
		CommitIntegration() {
			Config cfg {};
			_registry.setup(cfg);
			initActionResolver(_system, _resolver);
			if (GetParam()) {
				_io.start();
				_system.setIo(&_io);
			}

			auto lfsOwned = make_uptr<LocalFileSystemProvider>();
			_lfs = lfsOwned.get();
			_registry.addProvider(::hg::move(lfsOwned));
			_registry.addRepository(_lfs->makeCacheRepository(hg::fs::Path { _scratch.path }));
		}

	public:
		ScratchDir _scratch {};
		io::IoSystem _io {};
		StorageRegistry _registry {};
		StorageActionResolver _resolver {};
		StorageSystem _system { _registry, _resolver };
		ptr<LocalFileSystemProvider> _lfs = nullptr;

		ArchiveGuid _guid = ArchiveGuid::random();

	public:
		[[nodiscard]] Arci<PackageStorage> packageAt(ref<const std::filesystem::path> path_) {
			auto guid = Guid {};
			GuidGenerate(guid);
			auto file = _registry.insert(FileStorageDescriptor { FileUrl { "file+cache"sv, hg::fs::Path { path_ } } });
			return _registry.insert(PackageStorageDescriptor { PackageUrl { clone(guid) }, ::hg::move(file) }).into<PackageStorage>();
		}

		[[nodiscard]] static BufferArchive bufferOf(StringView data_) {
			auto buffer = BufferArchive {};
			buffer.reserve(data_.size());
			buffer.serializeBytes(const_cast<ptr<char>>(data_.data()), data_.size(), ArchiveStreamMode::eStore);
			buffer.setType(ArchiveType::eRaw);
			return buffer;
		}

		[[nodiscard]] auto link(ref<const Arci<PackageStorage>> pack_, ref<const ArchiveGuid> guid_, StringView data_) {
			auto buffer = bufferOf(data_);
			return _system.mutate(
				clone(pack_),
				[&](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
					auto& linked = rwp_.getLinker().add({ .type = ArchiveType::eUndefined, .guid = guid_ });
					linked.changes.emplace_back(resource::package::ArchiveDeltaAdd { .where = streamoff {}, .sizedData = buffer });
					return true;
				}
			);
		}

		[[nodiscard]] auto rewrite(ref<const Arci<PackageStorage>> pack_, ref<const ArchiveGuid> guid_, StringView data_) {
			auto buffer = bufferOf(data_);
			return _system.mutate(
				clone(pack_),
				[&](_Inout_ ref<ReadWritePackage> rwp_) -> bool {
					auto linked = rwp_.getLinker()[guid_];
					linked->changes.emplace_back(
						resource::package::ArchiveDeltaReplace { .where = streamoff {}, .byteCount = linked->data.size, .withSizedData = buffer }
					);
					return true;
				}
			);
		}

		[[nodiscard]] std::string read(ref<const Arci<PackageStorage>> pack_, ref<const ArchiveGuid> guid_) {
			auto bytes = std::string {};
			auto arch = _registry.insert(ArchiveStorageDescriptor { ArchiveUrl { guid_ }, clone(pack_).into<IStorage>() }).into<
				ArchiveStorage>();
			const auto result = _system.query(
				::hg::move(arch),
				[&](_In_ ref<StorageReadonlyArchive> arch_) {
					bytes.resize(static_cast<size_t>(arch_.totalSize()));
					arch_.seek(0);
					arch_.serializeBytes(bytes.data(), bytes.size(), ArchiveStreamMode::eLoad);
				}
			);
			EXPECT_TRUE(result.has_value());
			return bytes;
		}

		[[nodiscard]] static std::string fileBytes(ref<const std::filesystem::path> path_) {
			auto in = std::ifstream { path_, std::ios::binary };
			return std::string { std::istreambuf_iterator<char> { in }, std::istreambuf_iterator<char> {} };
		}

		[[nodiscard]] static bool hasTemp(ref<const std::filesystem::path> dir_) {
			for (const auto& entry : std::filesystem::directory_iterator { dir_ }) {
				if (entry.path().extension() == ::hg::engine::io::temp_suffix) {
					return true;
				}
			}
			return false;
		}
	};

	#if defined(_WIN32)
	TEST_P(CommitIntegration, LockedPackageReturnsCommitError) {
		const auto path = _scratch.path / "locked.imp";
		auto pack = packageAt(path);
		ASSERT_TRUE(link(pack, _guid, "old"sv).has_value());
		const auto before = fileBytes(path);

		const auto lock = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		ASSERT_NE(lock, INVALID_HANDLE_VALUE);
		auto failed = rewrite(pack, _guid, "new"sv);
		::CloseHandle(lock);

		ASSERT_FALSE(failed.has_value());
		ASSERT_TRUE(failed.error().is<io::CommitError>());
		const auto& error = failed.error().as<io::CommitError>();
		EXPECT_TRUE(error.kind == io::CommitError::Kind::eLocked || error.kind == io::CommitError::Kind::eAccessDenied);
		EXPECT_FALSE(error.path.empty());
		EXPECT_EQ(fileBytes(path), before);
		EXPECT_EQ(read(pack, _guid), "old");

		ASSERT_TRUE(rewrite(pack, _guid, "new"sv).has_value());
		EXPECT_EQ(read(pack, _guid), "new");
	}

	TEST_P(CommitIntegration, ReaderHandleOldQueryNew) {
		const auto path = _scratch.path / "reader.imp";
		auto pack = packageAt(path);
		ASSERT_TRUE(link(pack, _guid, "old-bytes"sv).has_value());
		const auto before = fileBytes(path);

		const auto reader = ::CreateFileW(
			path.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr
		);
		ASSERT_NE(reader, INVALID_HANDLE_VALUE);
		const auto done = rewrite(pack, _guid, "new-bytes"sv);

		auto seen = std::string(before.size() + 64uz, '\0');
		DWORD got = 0;
		const auto ok = ::ReadFile(reader, seen.data(), static_cast<DWORD>(seen.size()), &got, nullptr);
		::CloseHandle(reader);
		seen.resize(got);

		ASSERT_TRUE(done.has_value());
		EXPECT_TRUE(ok);
		EXPECT_EQ(seen, before);
		EXPECT_EQ(read(pack, _guid), "new-bytes");
	}

	TEST_P(CommitIntegration, FailedCreationLeavesNoUnparsableFile) {
		const auto path = _scratch.path / "created.imp";
		std::ofstream { path, std::ios::binary | std::ios::trunc }.close();

		const auto lock = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		ASSERT_NE(lock, INVALID_HANDLE_VALUE);
		auto pack = packageAt(path);
		auto failed = link(pack, _guid, "first"sv);
		::CloseHandle(lock);

		ASSERT_FALSE(failed.has_value());
		EXPECT_TRUE(failed.error().is<io::CommitError>());
		EXPECT_EQ(std::filesystem::file_size(path), 0uLL);
		EXPECT_FALSE(hasTemp(_scratch.path));
	}

	TEST_P(CommitIntegration, ArchiveMutateLockedThenSucceeds) {
		const auto path = _scratch.path / "archmut.imp";
		auto pack = packageAt(path);
		ASSERT_TRUE(link(pack, _guid, "old"sv).has_value());
		const auto before = fileBytes(path);

		auto arch = _registry.insert(ArchiveStorageDescriptor { ArchiveUrl { _guid }, clone(pack).into<IStorage>() }).into<ArchiveStorage>();
		const auto store = [&](StringView data_) {
			return _system.mutate(
				clone(arch),
				[&](_Inout_ ref<Archive> archive_) {
					archive_.seek(0);
					archive_.serializeBytes(const_cast<ptr<char>>(data_.data()), data_.size(), ArchiveStreamMode::eStore);
				}
			);
		};

		const auto lock = ::CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		ASSERT_NE(lock, INVALID_HANDLE_VALUE);
		auto failed = store("new"sv);
		::CloseHandle(lock);

		ASSERT_FALSE(failed.has_value());
		ASSERT_TRUE(failed.error().is<io::CommitError>());
		EXPECT_EQ(fileBytes(path), before);
		EXPECT_EQ(read(pack, _guid), "old");

		ASSERT_TRUE(store("new"sv).has_value());
		EXPECT_EQ(read(pack, _guid), "new");
	}
	#endif

	TEST_P(CommitIntegration, ManyPackagesInParallelStress) {
		constexpr auto packages = 8uz;
		constexpr auto rounds = 20uz;
		auto packs = Vector<Arci<PackageStorage>> {};
		auto guids = Vector<ArchiveGuid> {};
		for (auto i = 0uz; i < packages; ++i) {
			packs.push_back(packageAt(_scratch.path / std::format("many{}.imp", i)));
			guids.push_back(ArchiveGuid::random());
			ASSERT_TRUE(link(packs.back(), guids.back(), "00000000"sv).has_value());
		}

		auto failures = std::atomic_size_t { 0uz };
		auto workers = Vector<std::thread> {};
		for (auto i = 0uz; i < packages; ++i) {
			workers.emplace_back([&, i] {
				for (auto round = 1uz; round <= rounds; ++round) {
					if (not rewrite(packs[i], guids[i], std::format("{:08}", round * 10uz + i)).has_value()) {
						failures.fetch_add(1uz);
					}
				}
			});
		}
		for (auto& worker : workers) {
			worker.join();
		}

		EXPECT_EQ(failures.load(), 0uz);
		for (auto i = 0uz; i < packages; ++i) {
			EXPECT_EQ(read(packs[i], guids[i]), std::format("{:08}", rounds * 10uz + i));
		}
		EXPECT_FALSE(hasTemp(_scratch.path));
	}

	TEST_P(CommitIntegration, TwoPackagesInParallel) {
		auto first = packageAt(_scratch.path / "first.imp");
		auto second = packageAt(_scratch.path / "second.imp");
		const auto guidB = ArchiveGuid::random();

		bool okA = false;
		bool okB = false;
		auto a = std::thread { [&] { okA = link(first, _guid, "alpha"sv).has_value(); } };
		auto b = std::thread { [&] { okB = link(second, guidB, "beta"sv).has_value(); } };
		a.join();
		b.join();

		EXPECT_TRUE(okA);
		EXPECT_TRUE(okB);
		EXPECT_EQ(read(first, _guid), "alpha");
		EXPECT_EQ(read(second, guidB), "beta");
	}

	TEST_P(CommitIntegration, DiagnosticRendersSto0004) {
		const auto text = diag::render(
			storage::package::toDiagnostic(io::CommitError { .kind = io::CommitError::Kind::eDiskFull, .path = "x.imp" }, "x.imp")
		);
		EXPECT_NE(text.find("STO0004"), String::npos) << text;
		EXPECT_NE(text.find("disk-full"), String::npos) << text;
	}

	INSTANTIATE_TEST_SUITE_P(
		,
		CommitIntegration,
		testing::Bool(),
		[](ref<const testing::TestParamInfo<bool>> info_) { return info_.param ? "Io" : "Sync"; }
	);
}
