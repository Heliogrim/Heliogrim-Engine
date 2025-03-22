#include "DataWatcher.hpp"

using namespace hg::engine::reflow;
using namespace hg;

void DataWatcher::tick() {
	for (const auto& observer : _observers | std::views::values) {
		observer();
	}
}
