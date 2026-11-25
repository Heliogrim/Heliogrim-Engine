#include "Cell.hpp"

#include "../Widget/Widget.hpp"

import Engine.Asserts;

using namespace hg::engine::reflow;
using namespace hg;

namespace {
	void mergeReader(ref<Vector<CellReader>> readers_, ref<const CellReader> reader_);
}

void CellBase::bind(const nmpt<Widget> provider_, const type_id contextType_) noexcept {
	_provider = provider_;
	_contextType = contextType_;
}

nmpt<Widget> CellBase::provider() const noexcept {
	return _provider;
}

std::span<const CellReader> CellBase::readers() const noexcept {
	return _readers;
}

void CellBase::track(ref<Widget> reader_, const Flag<ReflowPhase, u8> phases_) {
	::hg::assertrt(not reader_.weak_from_this().expired());
	mergeReader(_readers, CellReader { reader_.weak_from_this(), phases_ });
}

void CellBase::notify() {
	// Readers added during the notification land in the emptied list and are merged back afterwards
	auto current = std::exchange(_readers, {});
	std::erase_if(
		current,
		[this](ref<const CellReader> reader_) {
			const auto widget = reader_.widget.lock();
			// A moved reader no longer resolves to this provider; dropping it is its unsubscription
			if (widget == nullptr || widget->contextProvider(_contextType) != _provider) {
				return true;
			}
			notifyAttributeChange(widget.get(), reader_.phases);
			return false;
		}
	);

	for (const auto& added : _readers) {
		mergeReader(current, added);
	}
	_readers = std::move(current);
}

/**/

namespace {
	void mergeReader(ref<Vector<CellReader>> readers_, ref<const CellReader> reader_) {
		const auto where = FindIf(
			readers_,
			[&](ref<const CellReader> existing_) {
				return not existing_.widget.owner_before(reader_.widget) && not reader_.widget.owner_before(existing_.widget);
			}
		);
		if (where != readers_.end()) {
			where->phases |= reader_.phases;
			return;
		}

		readers_.emplace_back(reader_);
	}
}
