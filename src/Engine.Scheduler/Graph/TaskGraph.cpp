#include "TaskGraph.hpp"

#include <algorithm>
#include <bitset>
#include <set>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

using namespace hg::engine::scheduler;
using namespace hg::engine::scheduler::graph;
using namespace hg;

namespace {
	using Bits = std::bitset<128>;

	[[nodiscard]] GraphError makeError(const GraphError::Kind kind_, Vector<string>&& stages_, Opt<AccessKey> key_ = {}) {
		return GraphError { .kind = kind_, .stages = std::move(stages_), .key = std::move(key_) };
	}

	[[nodiscard]] bool contains(const std::span<const AccessKey> keys_, ref<const AccessKey> key_) noexcept {
		return std::ranges::find(keys_, key_) != keys_.end();
	}

	[[nodiscard]] Opt<AccessKey> overlap(ref<const StageDecl> lhs_, ref<const StageDecl> rhs_) noexcept {
		for (const auto& key : lhs_.writes) {
			if (contains(rhs_.reads, key) || contains(rhs_.writes, key)) {
				return key;
			}
		}
		for (const auto& key : rhs_.writes) {
			if (contains(lhs_.reads, key)) {
				return key;
			}
		}
		return {};
	}
}

Result<Graph, GraphError> graph::build(const std::span<const StageDecl> decls_) {

	assertrt(decls_.size() <= 128uLL);
	const auto count = static_cast<u16>(decls_.size());

	/* 1. Name index */

	DenseMap<string_view, u16> index {};
	for (u16 idx = 0; idx < count; ++idx) {
		if (not index.emplace(decls_[idx].name, idx).second) {
			return Unexpected { makeError(GraphError::Kind::eDuplicateStage, { string { decls_[idx].name } }) };
		}
	}

	/* 2. One deduplicated edge set; `x before y` is `y after x` */

	Vector<Bits> edges(count);
	for (u16 idx = 0; idx < count; ++idx) {
		for (const auto& name : decls_[idx].after) {
			const auto iter = index.find(name);
			if (iter == index.end()) {
				return Unexpected { makeError(GraphError::Kind::eUnknownStage, { string { name } }) };
			}
			edges[iter->second].set(idx);
		}
		for (const auto& name : decls_[idx].before) {
			const auto iter = index.find(name);
			if (iter == index.end()) {
				return Unexpected { makeError(GraphError::Kind::eUnknownStage, { string { name } }) };
			}
			edges[idx].set(iter->second);
		}
	}

	/* 3. Kahn, ties broken by name */

	Vector<u16> indegree(count, 0);
	for (u16 from = 0; from < count; ++from) {
		for (u16 to = 0; to < count; ++to) {
			indegree[to] += edges[from][to];
		}
	}

	std::set<string_view> ready {};
	for (u16 idx = 0; idx < count; ++idx) {
		if (indegree[idx] == 0) {
			ready.insert(decls_[idx].name);
		}
	}

	Vector<u16> order {};
	order.reserve(count);
	while (not ready.empty()) {
		const auto from = index.at(*ready.begin());
		ready.erase(ready.begin());
		order.emplace_back(from);

		for (u16 to = 0; to < count; ++to) {
			if (edges[from][to] && --indegree[to] == 0) {
				ready.insert(decls_[to].name);
			}
		}
	}

	if (order.size() != count) {
		// Every remaining node keeps a remaining predecessor, so walking predecessors must repeat.
		auto cursor = static_cast<u16>(std::ranges::find_if(indegree, [](const auto deg_) { return deg_ > 0; }) - indegree.begin());

		Vector<u16> walk {};
		Bits seen {};
		while (not seen[cursor]) {
			seen.set(cursor);
			walk.emplace_back(cursor);

			for (u16 pred = 0; pred < count; ++pred) {
				if (edges[pred][cursor] && indegree[pred] > 0) {
					cursor = pred;
					break;
				}
			}
		}

		// The walk runs against the edges; the cycle starts at the repeated node.
		Vector<string> cycle {};
		const auto first = std::ranges::find(walk, cursor);
		for (auto iter = walk.rbegin(); iter != std::make_reverse_iterator(first); ++iter) {
			cycle.emplace_back(decls_[*iter].name);
		}
		return Unexpected { makeError(GraphError::Kind::eCycle, std::move(cycle)) };
	}

	/* 4. Reachability in reverse topological order */

	Vector<Bits> reach(count);
	for (auto iter = order.rbegin(); iter != order.rend(); ++iter) {
		for (u16 to = 0; to < count; ++to) {
			if (edges[*iter][to]) {
				reach[*iter] |= reach[to];
				reach[*iter].set(to);
			}
		}
	}

	/* 5. Conflicts between unordered pairs */

	for (size_t lhs = 0; lhs < order.size(); ++lhs) {
		for (size_t rhs = lhs + 1uLL; rhs < order.size(); ++rhs) {
			const auto a = order[lhs];
			const auto b = order[rhs];
			if (reach[a][b] || reach[b][a]) {
				continue;
			}

			if (auto key = overlap(decls_[a], decls_[b]); key.has_value()) {
				return Unexpected {
					makeError(
						GraphError::Kind::eConflict,
						{ string { decls_[a].name }, string { decls_[b].name } },
						std::move(key)
					)
				};
			}
		}
	}

	/* 6. + 7. Transitive reduction and emission */

	Graph result {};
	result.nodes.reserve(count);
	for (u16 idx = 0; idx < count; ++idx) {
		result.nodes.emplace_back(string { decls_[idx].name }, decls_[idx].lane, u16 { 0 }, Vector<u16> {});
	}

	for (u16 from = 0; from < count; ++from) {
		Bits implied {};
		for (u16 to = 0; to < count; ++to) {
			if (edges[from][to]) {
				implied |= reach[to];
			}
		}

		const auto reduced = edges[from] & ~implied;
		for (u16 to = 0; to < count; ++to) {
			if (reduced[to]) {
				result.nodes[from].successors.emplace_back(to);
				++result.nodes[to].predecessors;
			}
		}
	}

	result.order = std::move(order);
	return Expected { std::move(result) };
}

task::TaskMask graph::toMask(const Lane lane_) noexcept {
	return lane_ == Lane::eCritical ?
		task::TaskMask::eCritical :
		lane_ == Lane::eBackground ? task::TaskMask::eLower : task::TaskMask::eNormal;
}
