#pragma once

#include <gtest/gtest.h>

/**/

#include <Engine.Accel.Lang/Ir/Diagnostic.hpp>

namespace AccelLang {
	/* the message style rules: lower-case start (except proper nouns and quoted names), no trailing period; context notes start with `while` */
	inline void expectWording(hg::ref<const hg::engine::diag::Diagnostic> d_) {
		const auto checkText = [](hg::StringView text_, hg::StringView what_) {
			EXPECT_FALSE(text_.empty()) << what_;
			if (text_.empty()) {
				return;
			}
			const auto first = text_.front();
			const auto word = text_.substr(0uz, text_.find(' '));
			EXPECT_TRUE(first < 'A' || first > 'Z' || word == "GLSL" || word == "Vulkan") << what_ << ": '" << text_ << "' starts upper-case";
			EXPECT_NE(text_.back(), '.') << what_ << ": '" << text_ << "' ends in a period";
		};

		checkText(d_.message(), "message");
		for (const auto& label : d_.labels()) {
			checkText(label.message, "label");
		}
		for (const auto& note : d_.notes()) {
			EXPECT_FALSE(note.empty());
			EXPECT_NE(note.empty() ? ' ' : note.back(), '.') << "note: '" << note << "' ends in a period";
			if (note.starts_with("while") || note.starts_with("While")) {
				EXPECT_TRUE(note.starts_with("while ")) << "context note: '" << note << "'";
			}
		}
	}
}
