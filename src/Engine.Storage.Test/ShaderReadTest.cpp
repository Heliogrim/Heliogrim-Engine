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
}
