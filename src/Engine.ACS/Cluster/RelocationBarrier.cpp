#include "RelocationBarrier.hpp"

#include <algorithm>
#include <functional>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "ClusterRegion.hpp"

using namespace hg::engine::acs;
using namespace hg;

void RelocationBarrier::enqueue(MoveRequest request_) {
	_pending.emplace_back(request_);
}

void RelocationBarrier::discard(const ptr<const ClusterRegion> region_) {
	std::erase_if(_pending, [region_](ref<const MoveRequest> request_) { return request_.region == region_; });
}

Vector<RelocationTable> RelocationBarrier::apply() {
	// Moving a chunk erases it and shifts only higher indices, so higher chunks go first.
	std::ranges::sort(
		_pending,
		[](ref<const MoveRequest> left_, ref<const MoveRequest> right_) {
			return left_.region != right_.region ?
				std::less<ptr<ClusterRegion>> {}(left_.region, right_.region) :
				left_.chunk > right_.chunk;
		}
	);

	Vector<RelocationTable> tables {};
	tables.reserve(_pending.size());
	for (size_t index = 0uLL; index < _pending.size(); ++index) {
		const auto& request = _pending[index];
		const bool duplicate = index > 0uLL && _pending[index - 1uLL].region == request.region &&
			_pending[index - 1uLL].chunk == request.chunk;
		if (request.region == nullptr || duplicate || request.chunk >= request.region->chunkCount()) {
			continue;
		}
		tables.emplace_back(
			request.region->debugMoveChunk(
				request.chunk,
				[&request](const ptr<std::byte> src_, const ptr<std::byte> dst_) {
					request.relocate(src_, dst_, 1u, request.stride);
				}
			)
		);
	}
	_pending.clear();
	return tables;
}
