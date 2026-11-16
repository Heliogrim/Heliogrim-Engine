#include "Module.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <string>
#include <type_traits>

using namespace hg::engine::accel::lang;
using namespace hg;

ir::SourceSpan ir::spanIn(ref<const Module> mod_, SourceLoc loc_) {
	if (mod_.sources.empty()) {
		assert(loc_.source == 0u);
		return ir::pointAt(mod_.name, loc_);
	}
	assert(loc_.source < mod_.sources.size());
	return ir::pointAt(mod_.sources[loc_.source], loc_);
}

namespace {
	struct OpSpelling {
		ir::Op op;
		StringView text;
	};

	constexpr std::array<OpSpelling, 21uz> spellings {
		{
			{ ir::Op::eAdd, "+" }, { ir::Op::eSub, "-" }, { ir::Op::eMul, "*" }, { ir::Op::eDiv, "/" }, { ir::Op::eMod, "%" },
			{ ir::Op::eBitAnd, "&" }, { ir::Op::eBitOr, "|" }, { ir::Op::eBitXor, "^" }, { ir::Op::eShl, "<<" }, { ir::Op::eShr, ">>" },
			{ ir::Op::eLogicalAnd, "&&" }, { ir::Op::eLogicalOr, "||" }, { ir::Op::eEq, "==" }, { ir::Op::eNe, "!=" },
			{ ir::Op::eLt, "<" }, { ir::Op::eLe, "<=" }, { ir::Op::eGt, ">" }, { ir::Op::eGe, ">=" },
			{ ir::Op::eNeg, "-" }, { ir::Op::eNot, "!" }, { ir::Op::eBitNot, "~" }
		}
	};
}

StringView ir::spelling(const Op op_) noexcept {
	const auto it = std::ranges::find(spellings, op_, &OpSpelling::op);
	return it != spellings.end() ? it->text : StringView {};
}

ir::Op ir::opOf(const StringView text_) noexcept {
	const auto it = std::ranges::find(spellings, text_, &OpSpelling::text);
	return it != spellings.end() ? it->op : Op::eNone;
}

String ir::literalText(ref<const LiteralValue> literal_) {
	if (not literal_.spelling.empty()) {
		return literal_.spelling;
	}

	return std::visit(
		[](const auto& value_) -> String {
			using T = std::decay_t<decltype(value_)>;
			if constexpr (std::is_same_v<T, bool>) {
				return value_ ? "true" : "false";

			} else if constexpr (std::is_same_v<T, u64>) {
				return std::to_string(value_) + "u";

			} else if constexpr (std::is_same_v<T, s64>) {
				return std::to_string(value_);

			} else {
				std::array<char, 40uz> buffer {};
				const auto res = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value_);
				String out { buffer.data(), res.ptr };
				// GLSL needs a float marker; shortest round-trip text of 1000.0 is `1000`
				return out.find_first_of(".eEn") == String::npos ? out + ".0" : out;
			}
		},
		literal_.value
	);
}
