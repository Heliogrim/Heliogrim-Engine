#pragma once

#include <Engine.Accel.Lang/Ir/Diagnostic.hpp>
#include <gtest/gtest.h>

namespace AccelLang {
	/* the source span of `diag_`; any other location kind fails the test and yields an empty span */
	[[nodiscard]] inline hg::engine::accel::lang::ir::SourceSpan spanOf(hg::ref<const hg::engine::accel::lang::ir::Diagnostic> diag_) {
		const auto& primary = diag_.primary();
		EXPECT_TRUE(primary.is<hg::engine::accel::lang::ir::SourceSpan>()) << diag_.message();
		return primary.is<hg::engine::accel::lang::ir::SourceSpan>() ? primary.as<hg::engine::accel::lang::ir::SourceSpan>() : hg::engine::accel::lang::ir::SourceSpan {};
	}
}
