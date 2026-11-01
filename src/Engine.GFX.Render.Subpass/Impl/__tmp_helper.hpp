#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <Engine.Accel.Lang/Intermediate.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/Make.hpp>
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
			auto target = check / name_;
			if (std::filesystem::exists(target)) {
				inputFile = Some(::hg::move(target));
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

	/**
	 * Loads an `.asl` module below `shader/`; lowering to GLSL happens per stage when the effect is composed.
	 * Includes resolve below `shader/` as well.
	 */
	[[nodiscard]] inline smr<accel::lang::Intermediate> makeIrIntermediate(
		std::filesystem::path name_,
		accel::lang::ir::EntryKind entry_
	) {
		using namespace accel::lang;

		const auto name = name_.generic_string();
		auto parsed = ir::parseModule(
			name,
			read_shader_file(std::move(name_)),
			entry_,
			ir::ModuleResolver {
				[](StringView include_) -> Opt<String> {
					auto source = read_shader_file(std::filesystem::path { String { include_ } });
					return source.empty() ? None : Opt<String> { std::move(source) };
				}
			}
		);

		assert(parsed.has_value());
		auto intermediate = make_smr<Intermediate>();
		if (parsed.has_value()) {
			intermediate->ir = make_smr<const ir::Module>(new ir::Module { std::move(parsed).value() });
		}
		return intermediate;
	}
}
