#include "ClusterRegion.hpp"

#include <algorithm>
#include <bit>
#include <new>
#include <stdexcept>

using namespace hg;
using namespace hg::engine::acs;

namespace {
	constexpr u32 pageBytes = 4096u;
	constexpr u32 cacheLine = 64u;

	[[nodiscard]] std::align_val_t chunkAlign(const ClusterDesc& desc_) noexcept {
		return std::align_val_t { std::max(desc_.align, cacheLine) };
	}
}

ClusterRegion::ClusterRegion(const ClusterDesc desc_, ref<HandleTable> handles_) :
	_desc(desc_),
	_handles(handles_),
	// Large archetypes get a chunk of whole pages that holds at least one slot.
	_chunkSize(std::max(chunkBytes, (desc_.stride + pageBytes - 1u) / pageBytes * pageBytes)),
	_capacity(0u) {
	if (desc_.stride == 0u || not std::has_single_bit(desc_.align) || desc_.stride % desc_.align != 0u || desc_.actorOffset >= desc_.stride) {
		throw std::invalid_argument("ClusterRegion: stride must be a non-zero multiple of a power-of-two align and hold the actor offset");
	}
	_capacity = _chunkSize / desc_.stride;
}

ClusterRegion::~ClusterRegion() {
	releaseAll();
}

ClusterRegion::Slot ClusterRegion::claim(const Chunk* const exclude_) {
	Chunk* chunk = nullptr;
	// shortcut: linear scan for a non-full chunk, add a free-chunk list if regions grow past a few hundred chunks
	for (const auto& candidate : _chunks) {
		if (candidate.get() != exclude_ && candidate->live < _capacity) {
			chunk = candidate.get();
			break;
		}
	}

	if (chunk == nullptr) {
		auto* const base = static_cast<std::byte*>(::operator new(_chunkSize, chunkAlign(_desc)));
		const auto words = (_capacity + 63u) / 64u;
		chunk = _chunks.emplace_back(
			std::make_unique<Chunk>(Chunk { .base = base, .liveMask = std::vector<u64>(words, 0uLL), .handles = std::vector<Handle>(_capacity) })
		).get();
	}

	// The lowest free bit is below capacity because the chunk is not full.
	for (u32 word = 0u; ; ++word) {
		if (~chunk->liveMask[word] != 0uLL) {
			const auto bit = static_cast<u32>(std::countr_one(chunk->liveMask[word]));
			chunk->liveMask[word] |= 1uLL << bit;
			++chunk->live;
			++chunk->churn;
			return Slot { chunk, word * 64u + bit };
		}
	}
}

ClusterRegion::Slot ClusterRegion::find(const std::byte* const slot_) const {
	for (const auto& chunk : _chunks) {
		if (slot_ >= chunk->base && slot_ < chunk->base + static_cast<size_t>(_capacity) * _desc.stride) {
			return Slot { chunk.get(), static_cast<u32>(static_cast<size_t>(slot_ - chunk->base) / _desc.stride) };
		}
	}
	return Slot { nullptr, 0u };
}

void ClusterRegion::returnChunk(const Chunk* const chunk_) {
	const auto it = std::ranges::find(_chunks, chunk_, &std::unique_ptr<Chunk>::get);
	::operator delete(chunk_->base, _chunkSize, chunkAlign(_desc));
	_chunks.erase(it);
}

std::byte* ClusterRegion::allocate(Handle& out_) {
	const auto [chunk, index] = claim(nullptr);
	auto* const slot = chunk->base + static_cast<size_t>(index) * _desc.stride;
	out_ = _handles.acquire(slot + _desc.actorOffset);
	chunk->handles[index] = out_;
	return slot;
}

std::span<std::byte*> ClusterRegion::allocate(const size_t count_, const std::span<Handle> out_) {
	_batch.resize(count_);
	for (size_t i = 0uLL; i < count_; ++i) {
		_batch[i] = allocate(out_[i]);
	}
	return _batch;
}

void ClusterRegion::free(std::byte* const slot_) {
	const auto [chunk, index] = find(slot_);
	if (chunk == nullptr || not (chunk->liveMask[index / 64u] & (1uLL << (index % 64u)))) {
		return;
	}
	_handles.release(chunk->handles[index]);
	chunk->handles[index] = Handle {};
	chunk->liveMask[index / 64u] &= ~(1uLL << (index % 64u));
	++chunk->churn;
	if (--chunk->live == 0u) {
		returnChunk(chunk);
	}
}

void ClusterRegion::forEachChunk(const FnRef<void(std::byte*, u32, u32, const u64*)> fn_) const {
	for (const auto& chunk : _chunks) {
		fn_(chunk->base, _capacity, _desc.stride, chunk->liveMask.data());
	}
}

ChunkStats ClusterRegion::stats(const u32 chunk_) const {
	const auto& chunk = *_chunks[chunk_];
	u32 highest = 0u;
	for (u32 word = static_cast<u32>(chunk.liveMask.size()); word-- > 0u;) {
		if (chunk.liveMask[word] != 0uLL) {
			highest = word * 64u + 64u - static_cast<u32>(std::countl_zero(chunk.liveMask[word]));
			break;
		}
	}
	return ChunkStats { .live = chunk.live, .capacity = _capacity, .holes = highest - chunk.live, .churn = chunk.churn };
}

void ClusterRegion::releaseAll() {
	for (const auto& chunk : _chunks) {
		for (const auto handle : chunk->handles) {
			_handles.release(handle);
		}
		::operator delete(chunk->base, _chunkSize, chunkAlign(_desc));
	}
	_chunks.clear();
}

RelocationTable ClusterRegion::debugMoveChunk(const u32 chunk_, const FnRef<void(std::byte*, std::byte*)> relocate_) {
	auto* const source = _chunks[chunk_].get();
	std::vector<RelocationEntry> moved {};
	moved.reserve(source->live);

	for (u32 index = 0u; index < _capacity; ++index) {
		if (not (source->liveMask[index / 64u] & (1uLL << (index % 64u)))) {
			continue;
		}
		const auto [chunk, slot] = claim(source);
		auto* const from = source->base + static_cast<size_t>(index) * _desc.stride;
		auto* const to = chunk->base + static_cast<size_t>(slot) * _desc.stride;
		relocate_(from, to);

		chunk->handles[slot] = source->handles[index];
		_handles.rebind(chunk->handles[slot], to + _desc.actorOffset);
		moved.push_back(RelocationEntry { from, to });
	}

	returnChunk(source);
	return RelocationTable { _desc.stride, std::move(moved) };
}
