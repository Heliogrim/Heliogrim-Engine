#include "RootKind.hpp"

#include <format>
#include <limits>
#include <optional>
#include <Engine.Core/Engine.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "../SerializationModule.hpp"
#include "../Schema/Migration.hpp"
#include "../Structure/IntegralScopedSlot.hpp"
#include "../Structure/SlotCursor.hpp"

using namespace ::hg::engine::serialization;
using namespace ::hg;

namespace {
	/* Spec §10: a root `@type` must be registered; a SoA root is refused when its revision is newer, like a record. */
	[[nodiscard]] std::optional<RootKind> classifyStructRoot(ref<const StructScopedSlot> record_, ref<LoadContext> ctx_);
}

/**/

std::optional<RootKind> engine::serialization::classifyRoot(cref<RootScopedSlot> root_, ref<LoadContext> ctx_) {
	// A read-mode root carries no header until a typed view reads it, so peek it here.
	const auto& state = root_.slot()->getState();
	StructureSlotHeader header {};
	const auto total = state.root->archive->totalSize();
	if (auto error = readSlotHeader(
		*state.root->archive,
		SlotCursor { state.offset, total < 0 ? (std::numeric_limits<s64>::max)() : total },
		state.offset,
		header
	)) {
		ctx_.fail(::hg::move(*error));
		return std::nullopt;
	}

	switch (header.type) {
		case StructureSlotType::eSeq: {
			return RootKind::eAoS;
		}
		case StructureSlotType::eStruct: {
			return classifyStructRoot(root_.asStruct(), ctx_);
		}
		default: {
			ctx_.fail(
				ReadError {
					.kind = ReadError::Kind::eMalformed,
					.offset = root_.slot()->getState().offset,
					.length = 0LL,
					.detail = "root is neither a record, AoS nor SoA"
				}
			);
			return std::nullopt;
		}
	}
}

void engine::serialization::insertSoaRoot(ref<StructScopedSlot> root_, const type_id type_, const u64 count_) {
	root_.insertSlot<u64>(Symbols::Type) << type_.data;
	root_.insertSlot<u64>(soaCountKey) << count_;
	engine::Engine::getEngine()->getSerialization()->migrations().writeRevision(root_, type_);
}

/**/

namespace {
	std::optional<RootKind> classifyStructRoot(ref<const StructScopedSlot> record_, ref<LoadContext> ctx_) {
		const bool soa = record_.hasRecordSlot(soaCountKey);
		if (not record_.hasRecordSlot(Symbols::Type)) {
			return soa ? RootKind::eSoA : RootKind::eRecord;
		}

		type_id type {};
		record_.getSlot<u64>(Symbols::Type) >> type.data;
		const auto offset = record_.slot()->getState().offset;

		const auto serial = engine::Engine::getEngine()->getSerialization();
		if (not serial->getActorScheme(type).has_value() && not serial->getComponentScheme(type).has_value()) {
			ctx_.fail(
				ReadError {
					.kind = ReadError::Kind::eUnknownTypeRoot, .offset = offset, .length = 0LL, .detail = "root @type is not registered"
				}
			);
			return std::nullopt;
		}

		if (soa) {
			const u16 stored = record_.revision();
			const u16 current = serial->migrations().revision(type);
			if (stored > current) {
				ctx_.fail(
					ReadError {
						.kind = ReadError::Kind::eRevisionNewerRoot,
						.offset = offset,
						.length = 0LL,
						.detail = std::format("stored revision {} is newer than current revision {}", stored, current)
					}
				);
				return std::nullopt;
			}
		}

		return ctx_.failed() ? std::nullopt : std::optional { soa ? RootKind::eSoA : RootKind::eRecord };
	}
}
