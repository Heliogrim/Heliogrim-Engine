#pragma once

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <span>
#include <utility>
#include <Engine.Accel.Lang/Rep/Variable.hpp>
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
	 *
	 * `vertexOffsets_` maps vertex input ids to their byte offset in the vertex buffer; that layout is C++ data, so
	 * it is supplied here instead of being authored in the module. `vertexFormats_` likewise maps vertex input ids to
	 * the `vk::Format` of the stored attribute where it differs from the shader type (8-bit normalized data read as float).
	 */
	[[nodiscard]] inline smr<accel::lang::Intermediate> makeIrIntermediate(
		std::filesystem::path name_,
		accel::lang::ir::EntryKind entry_,
		std::span<const std::pair<String, u32>> vertexOffsets_ = {},
		std::span<const std::pair<String, s32>> vertexFormats_ = {}
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

		for (const auto& [id, offset] : vertexOffsets_) {
			const auto format = std::ranges::find_if(vertexFormats_, [&id](const auto& f_) { return f_.first == id; });
			auto var = make_uptr<accel::lang::Variable>();
			var->annotation = make_uptr<accel::lang::VkBindLocationAnnotation>(
				-1L,
				0L,
				static_cast<s32>(offset),
				make_uptr<accel::lang::SymbolIdAnnotation>(String { id }),
				format != vertexFormats_.end() ? format->second : -1L
			);
			intermediate->rep.globalScope.inbound.emplace_back(std::move(var));
		}

		return intermediate;
	}
}
