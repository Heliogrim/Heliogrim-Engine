#include "pch.h"

/**/

#include <Engine.Accel.Lang/Diagnostic/Codes.hpp>
#include <Engine.Diagnostics/Code.hpp>

import Engine.Common;
#include <Engine.Serialization/Diagnostic/Codes.hpp>
#include <Engine.Storage.Package/Diagnostic/Codes.hpp>

using namespace hg::engine::diag;
using namespace hg;

namespace HgDiagCodes {
	namespace {
		const std::array<std::span<const DiagnosticCodeInfo>, 3uLL> tables {
			std::span<const DiagnosticCodeInfo> { hg::engine::accel::lang::codes::table },
			std::span<const DiagnosticCodeInfo> { hg::engine::storage::package::codes::table },
			std::span<const DiagnosticCodeInfo> { hg::engine::serialization::codes::table }
		};

		String escaped(const StringView text_) {
			String out {};
			for (const char c : text_) {
				if (c == '"' || c == '\\') {
					out += '\\';
				}
				out += c;
			}
			return out;
		}

		void writeCodeList(ptr<const char> path_) {

			Vector<std::pair<String, ptr<const DiagnosticCodeInfo>>> entries {};
			for (const auto& table : tables) {
				for (const auto& info : table) {
					entries.emplace_back(formatCode(info.code), &info);
				}
			}
			std::ranges::sort(entries, [](const auto& l, const auto& r) { return l.first < r.first; });

			std::ofstream out { path_, std::ios::trunc };
			out << '[';
			bool first = true;
			for (const auto& [code, info] : entries) {
				out << (first ? "" : ",") << "{\"code\":\"" << code << "\",\"key\":\"" << escaped(info->key)
					<< "\",\"summary\":\"" << escaped(info->summary) << "\",\"slug\":\"" << escaped(slugOf(*info))
					<< "\"}";
				first = false;
			}
			out << "]\n";
		}
	}

	TEST(DiagnosticCodes, DomainsUnique) {
		for (const auto& a : tables) {
			for (const auto& b : tables) {
				if (a.data() == b.data()) {
					continue;
				}
				EXPECT_NE(a.front().code.domain, b.front().code.domain);
			}
		}
	}

	TEST(DiagnosticCodes, ExportWritesEveryCode) {
		writeCodeList("diagnostic-codes.json");

		std::ifstream in { "diagnostic-codes.json" };
		const std::string text { std::istreambuf_iterator<char> { in }, std::istreambuf_iterator<char> {} };

		size_t expected = 0;
		for (const auto& table : tables) {
			expected += table.size();
		}
		EXPECT_EQ(static_cast<size_t>(std::ranges::count(text, '{')), expected);
	}
}
