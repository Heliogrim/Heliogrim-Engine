#include "RelocationBarrier.hpp"

#include "ClusterRegion.hpp"

using namespace hg::engine::acs;
using namespace hg;

void RelocationBarrier::enqueue(MoveRequest request_) {
	_pending.push_back(request_);
}

std::vector<RelocationTable> RelocationBarrier::apply() {
	std::vector<RelocationTable> tables {};
	tables.reserve(_pending.size());
	for (const auto& request : _pending) {
		tables.push_back(
			request.region->debugMoveChunk(
				request.chunk,
				[&request](std::byte* const src_, std::byte* const dst_) {
					request.relocate(src_, dst_, 1u, request.stride);
				}
			)
		);
	}
	_pending.clear();
	return tables;
}
