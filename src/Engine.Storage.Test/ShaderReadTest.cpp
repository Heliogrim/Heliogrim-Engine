#include "pch.h"

/**/

#include "IntegrationFixture.hpp"

/**/

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <Engine.Storage.Registry/Provider/LocalFileSystemProvider.hpp>
#include <Engine.Storage.Registry/Repository/EditorReadOnlyRepository.hpp>
#include <Engine.Storage.Registry/Url/Scheme.hpp>
#include <Engine.Storage.System/ReadText.hpp>

/**/

using namespace hg::engine::storage;
using namespace hg;

namespace StorageModule {
	using ShaderReadTest = MutationIntegrationFixture;

	TEST_F(ShaderReadTest, ResolvesThroughStorage) {
		const auto root = std::filesystem::temp_directory_path() / std::format(
			"hg-shader-read-{}",
			std::chrono::steady_clock::now().time_since_epoch().count()
		);

		struct RemoveOnExit {
			std::filesystem::path path;

			~RemoveOnExit() {
				auto ec = std::error_code {};
				std::filesystem::remove_all(path, ec);
			}
		} cleanup { root };

		std::filesystem::create_directories(root / "shader");
		std::ofstream { root / "shader" / "probe.asl", std::ios::binary } << "probe";

		auto lfsOwned = make_uptr<system::LocalFileSystemProvider>();
		auto* const lfs = lfsOwned.get();
		_registry.addProvider(::hg::move(lfsOwned));
		_registry.addRepository(lfs->makeEditorRepository(fs::Path { root }));

		const auto probe = fs::Path { root / "shader" / "probe.asl" };
		EXPECT_EQ(readTextViaStorage(_system, FileEditorScheme, probe).value_or(""), "probe");
		// Note: A second read of the same url returns the same text.
		EXPECT_EQ(readTextViaStorage(_system, FileEditorScheme, probe).value_or(""), "probe");
		EXPECT_FALSE(readTextViaStorage(_system, FileEditorScheme, fs::Path { root / "shader" / "missing.asl" }).has_value());
	}

	class ShaderReadBadPathTest :
		public MutationIntegrationFixture {
	public:
		std::filesystem::path _root {};

		void SetUp() override {
			MutationIntegrationFixture::SetUp();

			_root = std::filesystem::temp_directory_path() / std::format(
				"hg-shader-bad-{}",
				std::chrono::steady_clock::now().time_since_epoch().count()
			);
			std::filesystem::create_directories(_root);

			auto lfsOwned = make_uptr<system::LocalFileSystemProvider>();
			auto* const lfs = lfsOwned.get();
			_registry.addProvider(::hg::move(lfsOwned));
			_registry.addRepository(lfs->makeEditorRepository(fs::Path { _root }));
		}

		void TearDown() override {
			auto ec = std::error_code {};
			std::filesystem::remove_all(_root, ec);
			MutationIntegrationFixture::TearDown();
		}

		void put(const std::filesystem::path& file_, std::string_view text_) const {
			std::ofstream { file_, std::ios::binary | std::ios::trunc } << text_;
		}
	};

	TEST_F(ShaderReadBadPathTest, MissingFileYieldsNone) {
		EXPECT_FALSE(readTextViaStorage(_system, FileEditorScheme, fs::Path { _root / "missing.asl" }).has_value());
	}

	TEST_F(ShaderReadBadPathTest, SchemeWithoutRepositoryYieldsNone) {
		const auto file = _root / "probe.asl";
		put(file, "probe");
		EXPECT_FALSE(readTextViaStorage(_system, UrlScheme { "file+nowhere"sv }, fs::Path { file }).has_value());
		// Note: The same file is still readable through the registered scheme.
		EXPECT_EQ(readTextViaStorage(_system, FileEditorScheme, fs::Path { file }).value_or(""), "probe");
	}

	TEST_F(ShaderReadBadPathTest, RelativePathYieldsNone) {
		const auto previous = std::filesystem::current_path();
		std::filesystem::current_path(_root);
		put(_root / "rel.asl", "rel");
		const auto result = readTextViaStorage(_system, FileEditorScheme, fs::Path { std::filesystem::path { "rel.asl" } });
		std::filesystem::current_path(previous);
		EXPECT_FALSE(result.has_value());
	}

	TEST_F(ShaderReadBadPathTest, EmptyFileYieldsEmptyText) {
		const auto file = _root / "empty.asl";
		put(file, "");
		// Note: Contract: an existing empty file is `Some("")`, not `None`.
		const auto result = readTextViaStorage(_system, FileEditorScheme, fs::Path { file });
		ASSERT_TRUE(result.has_value());
		EXPECT_TRUE(result.value().empty());
	}

	TEST_F(ShaderReadBadPathTest, DirectoryPathYieldsNone) {
		std::filesystem::create_directories(_root / "dir");
		EXPECT_FALSE(readTextViaStorage(_system, FileEditorScheme, fs::Path { _root / "dir" }).has_value());
	}

	TEST_F(ShaderReadBadPathTest, ReadOnlyFileYieldsContent) {
		const auto file = _root / "ro.asl";
		put(file, "readonly");
		std::filesystem::permissions(file, std::filesystem::perms::owner_read, std::filesystem::perm_options::replace);
		const auto result = readTextViaStorage(_system, FileEditorScheme, fs::Path { file });
		std::filesystem::permissions(file, std::filesystem::perms::owner_all, std::filesystem::perm_options::replace);
		EXPECT_EQ(result.value_or(""), "readonly");
	}

	TEST_F(ShaderReadBadPathTest, RewrittenFileIsReadFresh) {
		const auto file = _root / "live.asl";
		put(file, "first");
		ASSERT_EQ(readTextViaStorage(_system, FileEditorScheme, fs::Path { file }).value_or(""), "first");

		put(file, "second version");
		const auto second = readTextViaStorage(_system, FileEditorScheme, fs::Path { file });
		EXPECT_EQ(second.value_or("<none>"), "second version");
	}
}
