#include "Traits.hpp"

#include <atomic>
#include <chrono>

using namespace hg;

ActorGuid hg::generate_actor_guid() {
	static std::atomic_uint_fast32_t ai { 1 };
	const auto pre = ai.fetch_add(1, std::memory_order_relaxed);
	const auto post = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	return { pre, u16 { 0u }, u16 { 0u }, static_cast<u64>(post) };
}

ComponentGuid hg::generate_component_guid() {
	static std::atomic_uint_fast32_t ai { 1 };
	const auto pre = ai.fetch_add(1, std::memory_order_relaxed);
	const auto post = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	return { pre, u16 { 1u }, u16 { 1u }, static_cast<u64>(post) };
}
