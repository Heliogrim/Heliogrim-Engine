#include "Diagnostic.hpp"

import Engine.Asserts;
import Engine.Common;

using namespace hg::engine::diag;
using namespace hg;

Diagnostic::Diagnostic(ref<const Diagnostic> other_) :
	_data(other_._data ? make_uptr<Data>(*other_._data) : nullptr) {}

ref<Diagnostic> Diagnostic::operator=(ref<const Diagnostic> other_) {
	if (this != std::addressof(other_)) {
		_data = other_._data ? make_uptr<Data>(*other_._data) : nullptr;
	}
	return *this;
}

void Diagnostic::addNote(String note_) {
	assertrt(_data != nullptr);
	_data->notes.push_back(std::move(note_));
}

void Diagnostic::setMessage(String message_) {
	assertrt(_data != nullptr);
	_data->message = std::move(message_);
}

void Diagnostic::setPrimary(Location primary_) {
	assertrt(_data != nullptr);
	_data->primary = std::move(primary_);
}

bool Diagnostic::empty() const noexcept {
	return _data == nullptr;
}

ptr<const DiagnosticCodeInfo> Diagnostic::code() const noexcept {
	assertrt(_data != nullptr);
	return _data->code;
}

Severity Diagnostic::severity() const noexcept {
	assertrt(_data != nullptr);
	return _data->severity;
}

ref<const String> Diagnostic::message() const noexcept {
	assertrt(_data != nullptr);
	return _data->message;
}

ref<const Location> Diagnostic::primary() const noexcept {
	assertrt(_data != nullptr);
	return _data->primary;
}

ref<const Vector<Label>> Diagnostic::labels() const noexcept {
	assertrt(_data != nullptr);
	return _data->labels;
}

ref<const Vector<String>> Diagnostic::notes() const noexcept {
	assertrt(_data != nullptr);
	return _data->notes;
}

ref<const Vector<Fix>> Diagnostic::fixes() const noexcept {
	assertrt(_data != nullptr);
	return _data->fixes;
}

[[gnu::cold]] DiagnosticBuilder Diagnostic::error(ref<const DiagnosticCodeInfo> code_, String message_) {
	return DiagnosticBuilder { Severity::eError, code_, std::move(message_) };
}

[[gnu::cold]] DiagnosticBuilder Diagnostic::warning(ref<const DiagnosticCodeInfo> code_, String message_) {
	return DiagnosticBuilder { Severity::eWarning, code_, std::move(message_) };
}

[[gnu::cold]] DiagnosticBuilder Diagnostic::note(ref<const DiagnosticCodeInfo> code_, String message_) {
	return DiagnosticBuilder { Severity::eNote, code_, std::move(message_) };
}

[[gnu::cold]] DiagnosticBuilder Diagnostic::help(ref<const DiagnosticCodeInfo> code_, String message_) {
	return DiagnosticBuilder { Severity::eHelp, code_, std::move(message_) };
}

/**/

DiagnosticBuilder::DiagnosticBuilder(const Severity severity_, ref<const DiagnosticCodeInfo> code_, String message_) {
	_diagnostic._data = make_uptr<Diagnostic::Data>(
		Diagnostic::Data { std::addressof(code_), severity_, std::move(message_), NoLocation {}, {}, {}, {} }
	);
}

DiagnosticBuilder&& DiagnosticBuilder::at(Location location_) && {
	_diagnostic._data->primary = std::move(location_);
	return std::move(*this);
}

DiagnosticBuilder&& DiagnosticBuilder::label(Location location_, String message_) && {
	_diagnostic._data->labels.push_back(Label { std::move(location_), std::move(message_) });
	return std::move(*this);
}

DiagnosticBuilder&& DiagnosticBuilder::note(String note_) && {
	_diagnostic._data->notes.push_back(std::move(note_));
	return std::move(*this);
}

DiagnosticBuilder&& DiagnosticBuilder::fix(Fix fix_) && {
	_diagnostic._data->fixes.push_back(std::move(fix_));
	return std::move(*this);
}

DiagnosticBuilder::operator Diagnostic() && {
	return std::move(_diagnostic);
}

DiagnosticBuilder::operator Unexpected<Diagnostic>() && {
	return Unexpected<Diagnostic> { std::move(_diagnostic) };
}
