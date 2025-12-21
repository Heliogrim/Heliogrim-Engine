#include "SparseBuffer.hpp"

#include <algorithm>
#include <ranges>
#include <utility>

#include "SparseBufferView.hpp"
#include "../Command/CommandQueue.hpp"
#include "../Memory/VirtualMemory.hpp"

// TODO: Remove
#include <Engine.Core/Engine.hpp>

#include <Engine.Common/Discard.hpp>
#include <Engine.Common/Move.hpp>

#include "../Graphics.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SparseBuffer::SparseBuffer() noexcept :
	_bufferSize(0uLL),
	_memory(nullptr),
	_pages(),
	_vkBuffer(),
	_vkBufferUsageFlags(),
	_changed(false) {}

SparseBuffer::SparseBuffer(
	mref<uptr<VirtualMemory>> memory_,
	u64 bufferSize_,
	cref<vk::Buffer> buffer_,
	cref<vk::BufferUsageFlags> usageFlags_
) noexcept :
	_bufferSize(bufferSize_),
	_memory(std::move(memory_)),
	_pages(),
	_vkBuffer(buffer_),
	_vkBufferUsageFlags(usageFlags_),
	_changed(false) {}

SparseBuffer::SparseBuffer(mref<this_type> other_) noexcept :
	_bufferSize(std::exchange(other_._bufferSize, 0uLL)),
	_memory(std::move(other_._memory)),
	_pages(std::move(other_._pages)),
	_vkBuffer(std::exchange(other_._vkBuffer, {})),
	_vkBufferUsageFlags(std::exchange(other_._vkBufferUsageFlags, {})),
	_changed(std::exchange(other_._changed, false)),
	_bindings(std::move(other_._bindings)),
	_bindData(std::exchange(other_._bindData, {})) {}

SparseBuffer::~SparseBuffer() {
	tidy();
}

ref<SparseBuffer::this_type> SparseBuffer::operator=(mref<this_type> other_) noexcept {
	if (std::addressof(other_) != this) {
		/**
		 * Might be equal to `std::swap(*this, other_)`
		 */

		_bufferSize = std::exchange(other_._bufferSize, _bufferSize);
		_memory.swap(other_._memory);
		std::swap(_pages, other_._pages);
		_vkBuffer = std::exchange(other_._vkBuffer, _vkBuffer);
		_vkBufferUsageFlags = std::exchange(other_._vkBufferUsageFlags, _vkBufferUsageFlags);
		_changed = std::exchange(other_._changed, _changed);
		_bindings = std::exchange(other_._bindings, _bindings);
		_bindData = std::exchange(other_._bindData, _bindData);
	}

	return *this;
}

void SparseBuffer::tidy() {
	/**
	 * Destroy Buffer before backing memory or pages
	 */
	if (_vkBuffer) {
		// TODO: Check whether virtual buffer should get a reference to the device
		Engine::getEngine()->getGraphics()->getCurrentDevice()->vkDevice().destroyBuffer(_vkBuffer);
		_bufferSize = 0uLL;
	}

	/**
	 * Cleanup Pages
	 */
	for (auto& entry : _pages) {
		/* Memory Pages are getting freed by follow-up memory destructor */
		::hg::discard = entry->release();
		entry.reset();
	}

	_pages.clear();

	/**
	 * Cleanup Memory
	 */
	_memory.reset();
}

nmpt<const VirtualMemory> SparseBuffer::memory() const noexcept {
	return _memory.get();
}

nmpt<VirtualMemory> SparseBuffer::memory() noexcept {
	return _memory.get();
}

u64 SparseBuffer::size() const noexcept {
	return _bufferSize;
}

u64 SparseBuffer::memorySize() const noexcept {
	return _memory->size();
}

u64 SparseBuffer::memoryResidentialSize() const noexcept {
	return _memory->allocatedSize();
}

cref<vk::Buffer> SparseBuffer::vkBuffer() const noexcept {
	return _vkBuffer;
}

cref<vk::BufferUsageFlags> SparseBuffer::vkBufferUsageFlags() const noexcept {
	return _vkBufferUsageFlags;
}

nmpt<SparseBufferPage> SparseBuffer::addPage(const u64 size_, const u64 offset_) {
	#ifdef _DEBUG
	for (const auto& entry : _pages) {
		if (entry->resourceOffset() >= (offset_ + size_)) {
			continue;
		}

		const auto eo { entry->resourceOffset() };
		const auto es { entry->resourceSize() };

		assert((eo + es) <= offset_);
	}
	#endif

	// Warning: Replace virtual memory page offset
	const auto memPage = _memory->definePage(0uLL, size_);
	assert(memPage);

	auto page = make_uptr<SparseBufferPage>(
		memPage,
		size_,
		offset_
	);

	auto* const result = page.get();
	_pages.emplace_back(::hg::move(page));
	return result;
}

u64 SparseBuffer::getPageCount() const noexcept {
	return _pages.size();
}

void SparseBuffer::assureTiledPages(const u64 offset_, const u64 size_) {}

void SparseBuffer::selectPages(
	const u64 offset_,
	const u64 size_,
	ref<Vector<nmpt<SparseBufferPage>>> pages_
) {
	const u64 lowerBound { offset_ };
	const u64 upperBound { offset_ + size_ };

	for (auto& entry : _pages) {
		const u64 pageUpperBound { entry->resourceOffset() + entry->resourceSize() };

		if (entry->resourceOffset() > upperBound || lowerBound > pageUpperBound) {
			continue;
		}

		pages_.emplace_back(entry.get());
	}
}

uptr<SparseBufferView> SparseBuffer::makeView(const u64 offset_, const u64 size_) {
	assureTiledPages(offset_, size_);

	auto pages = Vector<nmpt<SparseBufferPage>> {};
	selectPages(offset_, size_, pages);

	/**
	 * Take the time to sort the pages by it's mip level (virtual backing)
	 */
	std::ranges::sort(
		pages,
		[](nmpt<SparseBufferPage> left_, nmpt<SparseBufferPage> right_) {
			return left_->resourceOffset() < right_->resourceOffset();
		}
	);

	/**/

	return uptr<SparseBufferView>(
		new SparseBufferView(
			*this,
			::hg::move(pages),
			offset_,
			size_
		)
	);
}

void SparseBuffer::updateBindingData() {

	const auto& updates = _pages;
	for (const auto& page : updates) {
		_bindings.emplace_back(page->vkSparseMemoryBind());
	}

	/**
	 *
	 */
	_bindData = vk::SparseBufferMemoryBindInfo {
		_vkBuffer,
		static_cast<u32>(_bindings.size()),
		_bindings.data()
	};
}

void SparseBuffer::enqueueBinding(const ptr<CommandQueue> queue_) {
	vk::BindSparseInfo bsi {
		0,
		nullptr,
		1uL,
		&_bindData,
		0,
		nullptr,
		0,
		nullptr,
		0,
		nullptr,
		nullptr
	};

	#ifdef _DEBUG
	const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
	assert(res == vk::Result::eSuccess);
	#else
	[[maybe_unused]] const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
	#endif
}

void SparseBuffer::enqueueBinding(
	const ptr<CommandQueue> queue_,
	cref<Vector<vk::Semaphore>> waits_,
	cref<Vector<vk::Semaphore>> signals_
) {
	vk::BindSparseInfo bsi {
		static_cast<u32>(waits_.size()),
		waits_.data(),
		1uL,
		&_bindData,
		0,
		nullptr,
		0,
		nullptr,
		static_cast<u32>(signals_.size()),
		signals_.data(),
		nullptr
	};

	#ifdef _DEBUG
	const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
	assert(res == vk::Result::eSuccess);
	#else
	[[maybe_unused]] const auto res { queue_->vkQueue().bindSparse(1, &bsi, nullptr) };
	#endif
}

void SparseBuffer::enqueueBindingSync(const ptr<CommandQueue> queue_) {
	vk::BindSparseInfo bsi {
		0,
		nullptr,
		1uL,
		&_bindData,
		0,
		nullptr,
		0,
		nullptr,
		0,
		nullptr,
		nullptr
	};

	auto fence { queue_->device()->vkDevice().createFence(vk::FenceCreateInfo {}) };

	#ifdef _DEBUG
	auto res { queue_->vkQueue().bindSparse(1, &bsi, fence) };
	assert(res == vk::Result::eSuccess);
	#else
	[[maybe_unused]] auto res { queue_->vkQueue().bindSparse(1, &bsi, fence) };
	#endif

	res = queue_->device()->vkDevice().waitForFences(1uL, &fence, VK_TRUE, UINT64_MAX);
	assert(res == vk::Result::eSuccess);

	queue_->device()->vkDevice().destroyFence(fence);
}