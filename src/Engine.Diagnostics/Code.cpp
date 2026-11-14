#include "Code.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <algorithm>
#include <format>

using namespace hg::engine;
using namespace hg;

String diag::slugOf(ref<const DiagnosticCodeInfo> info_) {
	auto slug = formatCode(info_.code);
	const auto letters = slug.size() - 4uLL;

	std::ranges::transform(slug, slug.begin(), [](const char c_) { return static_cast<char>(c_ >= 'A' && c_ <= 'Z' ? c_ + ('a' - 'A') : c_); });
	slug.insert(letters, "/");
	return slug;
}

String diag::formatCode(ref<const DiagnosticCode> code_) {
	assertrt(code_.number <= 9999);
	String out {};
	for (const auto c : code_.domain) {
		if (c == '\0') {
			break;
		}
		out.push_back(c);
	}
	return out.append(std::format("{:04}", code_.number));
}
