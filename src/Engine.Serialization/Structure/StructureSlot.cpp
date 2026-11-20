#include "StructureSlot.hpp"

#include <limits>
#include <Engine.Resource.Archive/Archive.hpp>

#include "SlotCursor.hpp"
#include "../Load/LoadContext.hpp"

using namespace hg::engine::serialization;
using namespace hg;

namespace {
	[[nodiscard]] nmpt<LoadContext> contextOf(ref<const StructureSlotState> state_) noexcept;

	/* Errors inside a migrated record point at the original record (record granularity). */
	[[nodiscard]] ReadError atOrigin(ref<const StructureSlotState> state_, mref<ReadError> error_) noexcept;

	/* 0 for bool, 1..4 for 8..64 bit; -1 for non-integral. */
	[[nodiscard]] s32 integralRank(const StructureSlotType type_) noexcept;

	[[nodiscard]] bool isSigned(const StructureSlotType type_) noexcept;
}

/**/

StructureSlotBase::~StructureSlotBase() = default;

cref<StructureSlotHeader> StructureSlotBase::getSlotHeader() const noexcept {
	return _state.header;
}

ref<StructureSlotHeader> StructureSlotBase::getSlotHeader() noexcept {
	return _state.header;
}

StructureSlotType StructureSlotBase::getSlotType() const noexcept {
	return StructureSlotType::eUndefined;
}

void StructureSlotBase::writeHeader() {

	auto archive = _state.root->archive;
	archive->seek(_state.offset);

	(*archive) << _state.header.type;

	if (not _state.header.sizeInferred) {
		(*archive) << _state.header.size;
	}
}

bool StructureSlotBase::readHeader() {

	if (_state.offset < 0 || not _state.root || not _state.root->archive) {
		_state.header = StructureSlotHeader {};
		return false;
	}

	if (const auto ctx = contextOf(_state); ctx && ctx->failed()) {
		_state.header = StructureSlotHeader {};
		return false;
	}

	if (not readHeaderAt(_state.offset, slotLimit(), _state.header)) {
		return false;
	}

	return true;
}

void StructureSlotBase::fail(const ReadError::Kind kind_, const s64 offset_, const s64 length_, ptr<const char> detail_) const {
	if (const auto ctx = contextOf(_state)) {
		ctx->fail(atOrigin(_state, ReadError { kind_, offset_, length_, String { detail_ } }));
	}
}

void StructureSlotBase::warn(const ReadError::Kind kind_, const s64 offset_, const s64 length_, String detail_) const {
	if (const auto ctx = contextOf(_state)) {
		ctx->warn(atOrigin(_state, ReadError { kind_, offset_, length_, ::hg::move(detail_) }));
	}
}

bool StructureSlotBase::hasPayload() const noexcept {
	return _state.offset >= 0 && _state.header.type != StructureSlotType::eUndefined &&
		_state.header.type != StructureSlotType::eRecord /* P10 placeholder reads as missing */ &&
		(_state.header.sizeInferred || _state.header.size >= 0);
}

s64 StructureSlotBase::payloadBegin() const noexcept {
	return _state.offset + static_cast<s64>(sizeof(StructureSlotType)) +
		(_state.header.sizeInferred ? 0LL : static_cast<s64>(sizeof(s64)));
}

s64 StructureSlotBase::payloadEnd() const noexcept {
	return payloadBegin() + (_state.header.size > 0 ? _state.header.size : 0LL);
}

s64 StructureSlotBase::slotSpan(ref<const StructureSlotHeader> header_) noexcept {
	return static_cast<s64>(sizeof(StructureSlotType)) + (header_.sizeInferred ? 0LL : static_cast<s64>(sizeof(s64))) +
		(header_.size > 0 ? header_.size : 0LL);
}

s64 StructureSlotBase::slotLimit() const noexcept {
	if (_state.parent && _state.parent->hasPayload()) {
		return _state.parent->payloadEnd();
	}
	const auto total = _state.root->archive->totalSize();
	return total < 0 ? (std::numeric_limits<s64>::max)() : total;
}

bool StructureSlotBase::readHeaderAt(const s64 at_, const s64 limit_, ref<StructureSlotHeader> out_) const {
	if (auto error = readSlotHeader(*_state.root->archive, SlotCursor { at_, limit_ }, at_, out_)) {
		if (const auto ctx = contextOf(_state)) {
			ctx->fail(atOrigin(_state, std::move(*error)));
		}
		return false;
	}
	return true;
}

bool StructureSlotBase::checkArchive(const s64 offset_, const s64 length_) const {
	if (_state.root->archive->hasError()) {
		fail(ReadError::Kind::eTruncated, offset_, length_, "read past the end of the archive");
		return false;
	}
	return true;
}

bool StructureSlotBase::charge(const u64 bytes_) const {
	const auto ctx = contextOf(_state);
	if (ctx && not ctx->charge(bytes_)) {
		fail(ReadError::Kind::eBudget, _state.offset, static_cast<s64>(bytes_), "allocation budget exceeded");
		return false;
	}
	return true;
}

bool StructureSlotBase::readIntegral(const StructureSlotType target_, ref<u64> raw_) {

	if (not hasPayload()) {
		return false;
	}

	const auto stored = _state.header.type;
	const auto sr = integralRank(stored);
	const auto tr = integralRank(target_);

	bool widen = stored == target_;
	if (not widen && sr > 0 && tr > 0) {
		widen = isSigned(stored) == isSigned(target_) ? tr >= sr : (not isSigned(stored) && tr > sr);
	}

	if (not widen) {
		fail(ReadError::Kind::eTypeMismatch, _state.offset, slotSpan(_state.header), "stored type does not widen to the target");
		return false;
	}

	auto archive = _state.root->archive;
	archive->seek(payloadBegin());

	u64 raw = 0;
	switch (sr) {
		case 0:
		case 1: {
			u8 v = 0;
			(*archive) >> v;
			raw = v;
			break;
		}
		case 2: {
			u16 v = 0;
			(*archive) >> v;
			raw = v;
			break;
		}
		case 3: {
			u32 v = 0;
			(*archive) >> v;
			raw = v;
			break;
		}
		default: {
			(*archive) >> raw;
			break;
		}
	}

	if (not checkArchive(_state.offset, slotSpan(_state.header))) {
		return false;
	}

	if (sr == 0 && raw > 1uLL) {
		fail(ReadError::Kind::eInvalidValue, _state.offset, slotSpan(_state.header), "bool is neither 0 nor 1");
		return false;
	}

	if (isSigned(stored) && sr < 4) {
		const auto shift = 64 - (8 << (sr - 1));
		raw = static_cast<u64>(static_cast<s64>(raw << shift) >> shift);
	}

	raw_ = raw;
	return true;
}

bool StructureSlotBase::readFloat(const StructureSlotType target_, ref<double> value_) {

	if (not hasPayload()) {
		return false;
	}

	const auto stored = _state.header.type;
	const bool widen = stored == target_ ||
		(stored == StructureSlotType::eFloat && target_ == StructureSlotType::eDouble);

	if (not widen) {
		fail(ReadError::Kind::eTypeMismatch, _state.offset, slotSpan(_state.header), "stored type does not widen to the target");
		return false;
	}

	auto archive = _state.root->archive;
	archive->seek(payloadBegin());

	if (stored == StructureSlotType::eFloat) {
		float v = 0.F;
		(*archive) >> v;
		value_ = v;
	} else {
		double v = 0.;
		(*archive) >> v;
		value_ = v;
	}

	return checkArchive(_state.offset, slotSpan(_state.header));
}

void StructureSlotBase::enter() {

	auto archive = _state.root->archive;

	if (_state.header.sizeInferred) {
		constexpr s64 off = sizeof(_state.header.type);
		archive->seek(_state.offset + off);

	} else {
		constexpr s64 off = sizeof(_state.header.type) + sizeof(_state.header.size);
		archive->seek(_state.offset + off);
	}
}

void StructureSlotBase::leave() {
	if (
		_state.flags & StructureSlotStateFlag::eMutable &&
		_state.flags & StructureSlotStateFlag::eDirty &&
		_state.parent
	) {
		_state.parent->feedback(this);
	}

	_state.flags.unwrap &= ~(static_cast<StructureSlotStateFlags::value_type>(StructureSlotStateFlag::eDirty));
}

void StructureSlotBase::feedback(const non_owning_rptr<const StructureSlotBase> other_) {}

/**/

namespace {
	nmpt<LoadContext> contextOf(ref<const StructureSlotState> state_) noexcept {
		return state_.root ? state_.root->context : nullptr;
	}

	ReadError atOrigin(ref<const StructureSlotState> state_, mref<ReadError> error_) noexcept {
		if (state_.root && state_.root->originOffset >= 0) {
			error_.offset = state_.root->originOffset;
			error_.length = state_.root->originLength;
		}
		return std::move(error_);
	}

	s32 integralRank(const StructureSlotType type_) noexcept {
		const auto code = static_cast<u8>(type_);
		if (code == static_cast<u8>(StructureSlotType::eBool)) {
			return 0;
		}
		return code >= static_cast<u8>(StructureSlotType::eU8) && code <= static_cast<u8>(StructureSlotType::eS64) ? code / 2 : -1;
	}

	bool isSigned(const StructureSlotType type_) noexcept {
		return integralRank(type_) > 0 && (static_cast<u8>(type_) & 1u) != 0u;
	}
}
