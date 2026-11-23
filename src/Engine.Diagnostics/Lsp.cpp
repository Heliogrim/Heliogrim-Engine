module;
module Engine.Diagnostics;

import Engine.Asserts;
import std;

import :Lsp;

using namespace hg::engine;
using namespace hg;

namespace {
	void quote(ref<String> out_, StringView text_);

	void position(ref<String> out_, const u32 line_, const u32 character_);

	void range(ref<String> out_, const u32 line_, const u32 from_, const u32 to_);

	void spanRange(ref<String> out_, ref<const diag::SourceSpan> span_);

	void diagnostic(ref<String> out_, ref<const diag::Diagnostic> d_);
}

String diag::toLspJson(ref<const Diagnostic> diagnostic_) {
	String out {};
	diagnostic(out, diagnostic_);
	return out;
}

String diag::toLspCodeActionsJson(ref<const Diagnostic> diagnostic_) {
	const auto json = toLspJson(diagnostic_);

	String out { "[" };
	bool firstAction = true;
	for (const auto& fix : diagnostic_.fixes()) {
		if (not firstAction) {
			out.push_back(',');
		}
		firstAction = false;

		out += R"({"title":)";
		quote(out, fix.title);
		out += R"(,"kind":"quickfix","diagnostics":[)" + json + R"(],"isPreferred":)";
		out += fix.applicability == Applicability::eMachineApplicable ? "true" : "false";
		out += R"(,"edit":{"changes":{)";

		/* group edits by uri, first-seen order */
		Vector<StringView> uris {};
		for (const auto& edit : fix.edits) {
			if (std::ranges::find(uris, StringView { edit.range.file }) == uris.end()) {
				uris.push_back(edit.range.file);
			}
		}
		for (size_t u = 0; u < uris.size(); ++u) {
			if (u > 0) {
				out.push_back(',');
			}
			quote(out, uris[u]);
			out += ":[";
			bool firstEdit = true;
			for (const auto& edit : fix.edits) {
				if (edit.range.file != uris[u]) {
					continue;
				}
				if (not firstEdit) {
					out.push_back(',');
				}
				firstEdit = false;
				out += R"({"range":)";
				spanRange(out, edit.range);
				out += R"(,"newText":)";
				quote(out, edit.replacement);
				out.push_back('}');
			}
			out.push_back(']');
		}
		out += "}}}";
	}
	out.push_back(']');
	return out;
}

String diag::lspUriOf(ref<const Diagnostic> diagnostic_) {
	const auto& primary = diagnostic_.primary();
	if (const auto* span = std::get_if<SourceSpan>(&primary)) {
		return span->file;
	}
	if (const auto* asset = std::get_if<AssetPath>(&primary)) {
		return asset->url;
	}
	if (const auto* blob = std::get_if<ByteOffset>(&primary)) {
		return blob->blob;
	}
	return {};
}

/**/

namespace {
	void quote(ref<String> out_, StringView text_) {
		out_.push_back('"');
		for (const auto c : text_) {
			switch (c) {
				case '"': out_ += "\\\""; break;
				case '\\': out_ += "\\\\"; break;
				case '\n': out_ += "\\n"; break;
				case '\t': out_ += "\\t"; break;
				case '\r': out_ += "\\r"; break;
				case '\b': out_ += "\\b"; break;
				case '\f': out_ += "\\f"; break;
				default:
					if (static_cast<unsigned char>(c) < 0x20) {
						out_ += std::format("\\u{:04x}", static_cast<unsigned>(c));
					} else {
						out_.push_back(c);
					}
			}
		}
		out_.push_back('"');
	}

	void position(ref<String> out_, const u32 line_, const u32 character_) {
		out_ += std::format(R"({{"line":{},"character":{}}})", line_, character_);
	}

	void range(ref<String> out_, const u32 line_, const u32 from_, const u32 to_) {
		out_ += R"({"start":)";
		position(out_, line_, from_);
		out_ += R"(,"end":)";
		position(out_, line_, to_);
		out_.push_back('}');
	}

	/* shortcut: the end stays on the begin line (single-line spans), upgrade when spans carry an end line and column */
	void spanRange(ref<String> out_, ref<const diag::SourceSpan> span_) {
		const auto line = span_.line > 0 ? span_.line - 1 : 0;
		const auto from = span_.column > 0 ? span_.column - 1 : 0;
		range(out_, line, from, from + (span_.end > span_.begin ? span_.end - span_.begin : 0u));
	}

	void diagnostic(ref<String> out_, ref<const diag::Diagnostic> d_) {
		out_ += R"({"range":)";
		if (const auto* span = std::get_if<diag::SourceSpan>(&d_.primary())) {
			spanRange(out_, *span);
		} else {
			range(out_, 0, 0, 0);
		}

		/* indexed by Severity */
		constexpr std::array<int, 4> lspSeverity { 4, 3, 2, 1 };
		const auto severityIndex = static_cast<size_t>(d_.severity());
		assertrt(severityIndex < lspSeverity.size());
		const auto& info = *d_.code();
		const auto code = diag::formatCode(info.code);
		const auto slug = diag::slugOf(info);

		out_ += std::format(R"(,"severity":{},"code":)", lspSeverity[severityIndex]);
		quote(out_, code);
		out_ += R"(,"codeDescription":{"href":)";
		quote(out_, String { diag::codeDocsBase } + slug);
		out_ += R"(},"source":)";

		auto source = String {};
		for (const auto c : info.code.domain) {
			if (c == '\0') {
				break;
			}
			source.push_back(static_cast<char>(c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c));
		}
		quote(out_, source);

		auto message = d_.message();
		for (const auto& note : d_.notes()) {
			message.push_back('\n');
			message += note;
		}
		out_ += R"(,"message":)";
		quote(out_, message);

		bool first = true;
		for (const auto& label : d_.labels()) {
			const auto* span = std::get_if<diag::SourceSpan>(&label.at);
			if (span == nullptr) {
				continue;
			}
			out_ += first ? R"(,"relatedInformation":[{"location":{"uri":)" : R"(,{"location":{"uri":)";
			first = false;
			quote(out_, span->file);
			out_ += R"(,"range":)";
			spanRange(out_, *span);
			out_ += R"(},"message":)";
			quote(out_, label.message);
			out_.push_back('}');
		}
		if (not first) {
			out_.push_back(']');
		}
		out_.push_back('}');
	}
}
