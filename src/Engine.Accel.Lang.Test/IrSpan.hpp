#pragma once

#include <Engine.Accel.Lang/Ir/Diagnostic.hpp>
#include <Engine.Diagnostics/Render.hpp>
#include <gtest/gtest.h>
#include <initializer_list>
#include <map>
#include <string>
#include <utility>

namespace AccelLang {
	/* file name to source text, for rendering diagnostics that span several modules */
	class ModuleSources final : public hg::engine::diag::SourceLookup {
	public:
		ModuleSources(std::initializer_list<std::pair<hg::StringView, hg::StringView>> files_) {
			for (const auto& [name, text] : files_) {
				_files.emplace(std::string(name), std::string(text));
			}
		}

		[[nodiscard]] hg::Optional<hg::StringView> find(hg::StringView file_) const override {
			const auto it = _files.find(std::string(file_));
			return it != _files.end() ? hg::Optional<hg::StringView> { hg::StringView { it->second } } : hg::Optional<hg::StringView> {};
		}

	private:
		std::map<std::string, std::string> _files;
	};

	/* the source span of `diag_`; any other location kind fails the test and yields an empty span */
	[[nodiscard]] inline hg::engine::accel::lang::ir::SourceSpan spanOf(hg::ref<const hg::engine::accel::lang::ir::Diagnostic> diag_) {
		const auto& primary = diag_.primary();
		EXPECT_TRUE(primary.is<hg::engine::accel::lang::ir::SourceSpan>()) << diag_.message();
		return primary.is<hg::engine::accel::lang::ir::SourceSpan>() ? primary.as<hg::engine::accel::lang::ir::SourceSpan>() : hg::engine::accel::lang::ir::SourceSpan {};
	}
}
