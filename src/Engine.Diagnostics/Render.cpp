#include "Render.hpp"

#include <algorithm>
#include <array>
#include <format>
import Engine.Asserts;

using namespace hg::engine;
using namespace hg;

namespace {
	void appendSnippet(ref<String> out_, ref<const diag::SourceSpan> span_, ptr<const diag::SourceLookup> sources_);

	void appendLocation(ref<String> out_, ref<const diag::Location> at_, StringView message_, ptr<const diag::SourceLookup> sources_);
}

StringView diag::severityName(const Severity severity_) {
	constexpr std::array<StringView, 4> names { StringView {"help"}, "note", "warning", "error" };
	return names.at(static_cast<size_t>(severity_));
}

String diag::render(ref<const Diagnostic> diagnostic_, ptr<const SourceLookup> sources_) {
	assertrt(not diagnostic_.empty());

	String out = std::format(
		"{}[{}]: {}\n",
		severityName(diagnostic_.severity()),
		formatCode(diagnostic_.code()->code),
		diagnostic_.message()
	);

	appendLocation(out, diagnostic_.primary(), {}, sources_);
	for (const auto& label : diagnostic_.labels()) {
		appendLocation(out, label.at, label.message, sources_);
	}
	for (const auto& label : diagnostic_.labels()) {
		if (label.at.is<NoLocation>()) {
			out.append("  = ").append(label.message).push_back('\n');
		}
	}
	for (const auto& note : diagnostic_.notes()) {
		out.append("  = note: ").append(note).push_back('\n');
	}
	for (const auto& fix : diagnostic_.fixes()) {
		out.append("  = help: ").append(fix.title).push_back('\n');
	}
	return out;
}

/**/

namespace {
	void appendSnippet(ref<String> out_, ref<const diag::SourceSpan> span_, ptr<const diag::SourceLookup> sources_) {
		if (sources_ == nullptr || span_.line == 0) {
			return;
		}

		const auto source = sources_->find(span_.file);
		if (not source) {
			return;
		}

		/* The span's line is the line-th '\n' separated line of the source; absent lines fall back to no snippet. */
		StringView rest = *source;
		for (u32 idx = 1; idx < span_.line; ++idx) {
			const auto nl = rest.find('\n');
			if (nl == StringView::npos) {
				return;
			}
			rest.remove_prefix(nl + 1);
		}
		if (rest.empty()) {
			return;
		}

		StringView text = rest.substr(0, rest.find('\n'));
		if (text.ends_with('\r')) {
			text.remove_suffix(1);
		}

		const auto col = static_cast<size_t>(span_.column > 0 ? span_.column - 1 : 0);
		const auto start = std::min(col, text.size());
		const auto width = std::clamp<size_t>(span_.end > span_.begin ? span_.end - span_.begin : 1, 1, std::max<size_t>(text.size() - start, 1));

		const auto gutter = std::to_string(span_.line).size();
		out_.append(std::format("{:>{}} |\n", "", gutter));
		out_.append(std::format("{} | {}\n", span_.line, text));
		out_.append(std::format("{:>{}} | {}{}\n", "", gutter, String(start, ' '), String(width, '^')));
	}

	void appendLocation(
		ref<String> out_,
		ref<const diag::Location> at_,
		StringView message_,
		ptr<const diag::SourceLookup> sources_
	) {
		String head {};
		if (at_.is<diag::SourceSpan>()) {
			const auto& s = at_.as<diag::SourceSpan>();
			head = std::format("{}:{}:{}", s.file, s.line, s.column);
		} else if (at_.is<diag::AssetPath>()) {
			const auto& a = at_.as<diag::AssetPath>();
			head = std::format("asset {}{}{}", a.url, a.recordPath.empty() ? "" : "#", a.recordPath);
		} else if (at_.is<diag::ActorPath>()) {
			const auto& a = at_.as<diag::ActorPath>();
			head = std::format("actor {}/{}", a.actor, a.componentPath);
		} else if (at_.is<diag::ByteOffset>()) {
			const auto& b = at_.as<diag::ByteOffset>();
			head = std::format("blob {} @ {}+{}", b.blob, b.offset, b.length);
		} else {
			return;
		}

		out_.append("  --> ").append(head);
		if (not message_.empty()) {
			out_.append(": ").append(message_);
		}
		out_.push_back('\n');

		if (at_.is<diag::SourceSpan>()) {
			appendSnippet(out_, at_.as<diag::SourceSpan>(), sources_);
		}
	}
}
