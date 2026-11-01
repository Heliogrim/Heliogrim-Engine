#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>

namespace AccelLang {
	/* `.asl` files shipped under `assets/shader`; modules are named by their path below it */
	[[nodiscard]] inline hg::Opt<hg::String> readShaderAsset(hg::StringView name_) {
		const auto root = std::filesystem::path { __FILE__ }.parent_path().parent_path().parent_path() / "assets" / "shader";
		std::ifstream file { root / std::string { name_ }, std::ios::in | std::ios::binary };
		if (not file.is_open()) {
			return hg::None;
		}
		std::ostringstream text {};
		text << file.rdbuf();
		return hg::String { text.str() };
	}

	/* parsed from disk with includes resolved below `assets/shader`, then resolved with default toggles */
	[[nodiscard]] inline hg::engine::accel::lang::ir::Module shaderAssetModule(
		hg::StringView name_,
		hg::engine::accel::lang::ir::EntryKind entry_
	) {
		using namespace hg::engine::accel::lang::ir;

		const auto source = readShaderAsset(name_);
		EXPECT_TRUE(source.has_value()) << "missing shader asset " << std::string { name_ };
		if (not source.has_value()) {
			return {};
		}

		auto parsed = parseModule(name_, *source, entry_, ModuleResolver { readShaderAsset });
		EXPECT_TRUE(parsed.has_value()) << (parsed.has_value() || parsed.error().empty() ? "" : parsed.error().front().message);
		if (not parsed.has_value()) {
			return {};
		}

		auto resolved = resolveToggles(*parsed, ToggleSet {});
		EXPECT_TRUE(resolved.has_value());
		return resolved.has_value() ? std::move(resolved).value() : Module {};
	}
}
