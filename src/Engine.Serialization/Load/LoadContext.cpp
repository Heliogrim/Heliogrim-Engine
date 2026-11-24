#include "LoadContext.hpp"

#include <algorithm>

#include "../Diagnostic/Codes.hpp"

using namespace hg::engine;
using namespace hg::engine::serialization;
using namespace hg;

LoadContext::LoadContext(const u64 archiveSize_) :
	_budget(std::max(archiveSize_ * 4uLL, minBudget)) {}

void LoadContext::fail(ReadError error_) {
	if (_failed) {
		_notes.emplace_back(Entry { ::hg::move(error_), recordPath() });
		return;
	}

	_failed = true;
	_first = Entry { ::hg::move(error_), recordPath() };
}

bool LoadContext::failed() const noexcept {
	return _failed;
}

ptr<const ReadError> LoadContext::firstError() const noexcept {
	return _failed ? &_first.error : nullptr;
}

void LoadContext::warn(ReadError error_) {
	_warnings.push_back(Entry { ::hg::move(error_), recordPath() });
}

void LoadContext::pushPath(String segment_) {
	_path.push_back(::hg::move(segment_));
}

void LoadContext::popPath() {
	if (not _path.empty()) {
		_path.pop_back();
	}
}

String LoadContext::recordPath() const {
	String out {};
	for (const auto& segment : _path) {
		if (not out.empty()) {
			out += " / ";
		}
		out += segment;
	}
	return out;
}

bool LoadContext::enterDepth() {
	if (_depth >= maxDepth) {
		return false;
	}

	++_depth;
	return true;
}

void LoadContext::leaveDepth() {
	if (_depth > 0) {
		--_depth;
	}
}

bool LoadContext::charge(const u64 bytes_) {
	if (bytes_ > _budget) {
		return false;
	}

	_budget -= bytes_;
	return true;
}

void LoadContext::markRead(ptr<const void> archive_, const s64 structOffset_, const StringView key_) {
	_read[read_key { archive_, structOffset_ }].emplace(key_);
}

void LoadContext::forgetArchive(ptr<const void> archive_) {
	std::erase_if(_read, [archive_](const auto& entry_) { return entry_.first.first == archive_; });
}

Vector<String> LoadContext::unreadKeys(ptr<const void> archive_, const s64 structOffset_, std::span<const String> present_) const {
	const auto it = _read.find(read_key { archive_, structOffset_ });
	Vector<String> out {};
	for (const auto& key : present_) {
		if (it == _read.end() || not it->second.contains(key)) {
			out.emplace_back(key);
		}
	}
	return out;
}

namespace {
	struct Mapped {
		ref<const diag::DiagnosticCodeInfo> code;
		bool root;
		bool bytes;
	};

	Mapped mapKind(const ReadError::Kind kind_) {
		using enum ReadError::Kind;
		switch (kind_) {
			case eMalformed: return { codes::ser("malformed-slot"), false, true };
			case eTruncated: return { codes::ser("truncated"), false, true };
			case eDepth: return { codes::ser("nesting-too-deep"), false, true };
			case eBudget: return { codes::ser("limit-exceeded"), false, true };
			case eTypeMismatch: return { codes::ser("type-mismatch"), false, false };
			case eInvalidValue: return { codes::ser("invalid-value"), false, false };
			case eDuplicateKey: return { codes::ser("duplicate-key"), false, false };
			case eMissingField: return { codes::ser("missing-field"), false, false };
			case eDuplicateIdentity: return { codes::ser("duplicate-identity"), false, false };
			case eUnknownTypeRoot: return { codes::ser("unknown-type"), true, false };
			case eRevisionNewerRoot: return { codes::ser("revision-newer"), true, false };
			case eUnknownType: return { codes::ser("unknown-type"), false, false };
			case eUnknownField: return { codes::ser("unknown-field"), false, false };
			case eRevisionNewer: return { codes::ser("revision-newer"), false, false };
			case eUnresolvedReference: return { codes::ser("unresolved-reference"), false, false };
			case eMigrationFailed: default: return { codes::ser("migration-failed"), false, false };
		}
	}

	diag::Diagnostic toDiagnostic(cref<ReadError> error_, String path_, const bool asError_) {

		const auto mapped = mapKind(error_.kind);
		String message = error_.detail.empty() ? String { mapped.code.summary } : error_.detail;

		diag::Location at = mapped.bytes ?
			diag::Location { diag::ByteOffset { ::hg::move(path_), static_cast<u64>(error_.offset), static_cast<u64>(error_.length) } } :
			diag::Location { diag::AssetPath { String {}, ::hg::move(path_) } };

		return asError_ ?
			diag::Diagnostic::error(mapped.code, ::hg::move(message)).at(::hg::move(at)) :
			diag::Diagnostic::warning(mapped.code, ::hg::move(message)).at(::hg::move(at));
	}
}

Result<void, diag::Diagnostics> LoadContext::finish(
	ref<diag::DiagnosticSink> sink_,
	const StringView contextNote_
) {
	for (auto& entry : _warnings) {
		auto diagnostic = toDiagnostic(entry.error, ::hg::move(entry.path), false);
		diagnostic.addNote(String { contextNote_ });
		sink_.emit(::hg::move(diagnostic));
	}
	_warnings.clear();

	if (not _failed) {
		return {};
	}

	auto diagnostic = toDiagnostic(_first.error, _first.path, true);
	for (const auto& note : _notes) {
		diagnostic.addNote(String { mapKind(note.error.kind).code.key } + (note.error.detail.empty() ? "" : ": " + note.error.detail));
	}
	diagnostic.addNote(String { contextNote_ });
	_notes.clear();
	_failed = false;

	diag::Diagnostics out {};
	out.push_back(::hg::move(diagnostic));
	return Unexpected<diag::Diagnostics> { ::hg::move(out) };
}
