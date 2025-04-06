#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Core/Engine.hpp>

namespace hg::engine::render {
	inline static string read_shader_file(std::filesystem::path name_) {

		const auto& cfg = Engine::getEngine()->getConfig();
		auto checks = Vector<std::filesystem::path> {};

		const auto& projectPath = cfg.getTyped<String>(cfg::ProjectConfigProperty::eLocalAssetPath);
		if (projectPath.has_value() && projectPath->has_value()) {
			checks.emplace_back(std::filesystem::path { **projectPath }.append(R"(shader)").lexically_normal());
		}

		const auto& editorPath = cfg.getTyped<String>(cfg::EditorConfigProperty::eLocalAssetPath);
		if (editorPath.has_value() && editorPath->has_value()) {
			checks.emplace_back(std::filesystem::path { **editorPath }.append(R"(shader)").lexically_normal());
		}

		Opt<std::filesystem::path> inputFile = None;
		for (const auto& check : checks) {
			const auto target = check / name_;
			if (std::filesystem::exists(target)) {
				inputFile = Some(target);
				break;
			}
		}

		if (inputFile == None) {
			::hg::breakpoint();
			return {};
		}

		/**/

		auto ifs = std::ifstream(*inputFile, std::ios_base::in | std::ios_base::binary);

		ifs.seekg(0, std::ios::end);
		const auto fsize = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		string tmp {};
		tmp.resize(fsize);

		ifs.read(tmp.data(), fsize);
		assert(!ifs.bad());

		ifs.close();
		return tmp;
	}
}
