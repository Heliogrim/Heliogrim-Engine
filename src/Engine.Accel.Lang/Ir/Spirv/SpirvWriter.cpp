#include "SpirvWriter.hpp"

using namespace hg::engine::accel::lang::ir::spirv;
using namespace hg;

u32 SpirvWriter::id() noexcept {
	return _bound++;
}

#define HG_SPIRV_SECTION(fn_, member_) \
	void SpirvWriter::fn_(const spv::Op op_, const std::span<const u32> operands_) { append(member_, op_, operands_); }

HG_SPIRV_SECTION(capability, _capability)
HG_SPIRV_SECTION(extension, _extension)
HG_SPIRV_SECTION(extInstImport, _extInstImport)
HG_SPIRV_SECTION(memoryModel, _memoryModel)
HG_SPIRV_SECTION(entryPoint, _entryPoint)
HG_SPIRV_SECTION(executionMode, _executionMode)
HG_SPIRV_SECTION(debug, _debug)
HG_SPIRV_SECTION(name, _name)
HG_SPIRV_SECTION(annotation, _annotation)
HG_SPIRV_SECTION(global, _global)
HG_SPIRV_SECTION(function, _function)

#undef HG_SPIRV_SECTION

u32 SpirvWriter::type(const spv::Op op_, const std::span<const u32> operands_, const bool fresh_) {

	Vector<u32> key { static_cast<u32>(op_) };
	key.insert(key.end(), operands_.begin(), operands_.end());
	if (not fresh_) {
		if (const auto it = _dedup.find(key); it != _dedup.end()) {
			return it->second;
		}
	}

	const auto result = id();
	Vector<u32> words { result };
	words.insert(words.end(), operands_.begin(), operands_.end());
	append(_global, op_, words);
	if (not fresh_) {
		_dedup.emplace(std::move(key), result);
	}
	return result;
}

u32 SpirvWriter::constant(
	const spv::Op op_,
	const u32 resultType_,
	const std::span<const u32> operands_,
	const bool fresh_
) {

	Vector<u32> key { static_cast<u32>(op_), resultType_ };
	key.insert(key.end(), operands_.begin(), operands_.end());
	if (not fresh_) {
		if (const auto it = _dedup.find(key); it != _dedup.end()) {
			return it->second;
		}
	}

	const auto result = id();
	Vector<u32> words { resultType_, result };
	words.insert(words.end(), operands_.begin(), operands_.end());
	append(_global, op_, words);
	if (not fresh_) {
		_dedup.emplace(std::move(key), result);
	}
	return result;
}

void SpirvWriter::string(ref<Vector<u32>> out_, const StringView value_) {
	const auto words = (value_.size() + 1uz + 3uz) / 4uz;
	const auto first = out_.size();
	out_.resize(first + words, 0uL);
	for (auto i = 0uz; i < value_.size(); ++i) {
		out_[first + i / 4uz] |= static_cast<u32>(static_cast<u8>(value_[i])) << (8uL * static_cast<u32>(i % 4uz));
	}
}

Vector<u32> SpirvWriter::finish() const {
	// magic, version 1.6, generator 0, bound, schema 0
	Vector<u32> out { spv::MagicNumber, spv::Version, 0uL, _bound, 0uL };
	for (const auto* section : { &_capability, &_extension, &_extInstImport, &_memoryModel, &_entryPoint, &_executionMode, &_debug,
		     &_name, &_annotation, &_global, &_function }) {
		out.insert(out.end(), section->begin(), section->end());
	}
	return out;
}

bool SpirvWriter::overlong() const noexcept {
	return _overlong;
}

void SpirvWriter::append(ref<Vector<u32>> into_, const spv::Op op_, const std::span<const u32> operands_) {
	if (operands_.size() >= 0xFFFFuz) {
		_overlong = true;
		return;
	}

	into_.push_back((static_cast<u32>(operands_.size() + 1uz) << 16uL) | static_cast<u32>(op_));
	into_.insert(into_.end(), operands_.begin(), operands_.end());
}
