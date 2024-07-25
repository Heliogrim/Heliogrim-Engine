#include "GlobalResourcePool.hpp"

#include <Engine.Asserts/Breakpoint.hpp>

#include "../Buffer/SparseBufferView.hpp"
#include "../Texture/SparseTextureView.hpp"
#include "../Texture/TextureFactory.hpp"
#include "Engine.GFX/Geometry/Vertex.hpp"

using namespace hg::engine::gfx::pool;
using namespace hg;

GlobalResourcePool::GlobalResourcePool(cref<sptr<Device>> device_) :
	_device(device_),
	_globalVertexBuffer(device_),
	_globalIndexBuffer(device_),
	_atlasMaxLayers(128uL),
	_atlasMaxLayerExtent({ 8192uL }),
	_atlasMinLayerExtent({ 16uL }),
	_atlasMaxSpatialExtent({ 0uL }),
	_atlasMinSpatialExtent({ 0uL }) {}

GlobalResourcePool::~GlobalResourcePool() = default;

void GlobalResourcePool::setup() {

	constexpr auto preAllocatedVertexSize = 1024uLL * 1024uLL * 1024uLL * sizeof(vertex);
	constexpr auto preAllocatedIndexSize = 1024uLL * 1024uLL * 1024uLL * sizeof(u32);

	/**/

	assert(_vertexBuffers.empty());
	_globalVertexBuffer._vkBufferUsage = vk::BufferUsageFlagBits::eVertexBuffer
		| vk::BufferUsageFlagBits::eTransferSrc
		| vk::BufferUsageFlagBits::eTransferDst;
	_globalVertexBuffer.setup(preAllocatedVertexSize);

	/**/

	assert(_indexBuffers.empty());

	_globalIndexBuffer._vkBufferUsage = vk::BufferUsageFlagBits::eIndexBuffer
		| vk::BufferUsageFlagBits::eTransferSrc
		| vk::BufferUsageFlagBits::eTransferDst;
	_globalIndexBuffer.setup(preAllocatedIndexSize);
}

void GlobalResourcePool::destroy() {

	/**
	 * Release unique pointers to virtual buffers for vertices
	 */
	for (auto& entry : _vertexBuffers) {
		entry.reset();
	}

	/**
	 * Release unique pointers to virtual buffers for indices
	 */
	for (auto& entry : _indexBuffers) {
		entry.reset();
	}

	/**
	 * Clear lists
	 */
	_indexBuffers.clear();
	_vertexBuffers.clear();

	/**/

	_globalVertexBuffer.destroy();
	_globalIndexBuffer.destroy();
}

cref<sptr<engine::gfx::Device>> GlobalResourcePool::device() const noexcept {
	return _device;
}

uptr<engine::gfx::SparseBufferView> GlobalResourcePool::allocateIndexBuffer(
	mref<IndexBufferAllocation> allocation_
) {

	assert(allocation_.size > 0uLL);
	auto result = _globalIndexBuffer.acquire(allocation_.size);
	if (result != nullptr) {
		return result;
	}

	::hg::breakpoint();

	/**/

	/**
	 * Create sparse vulkan buffer
	 */
	vk::BufferCreateInfo ci {
		vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency,
		allocation_.size,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::SharingMode::eExclusive,
		0uL,
		nullptr,
		nullptr
	};

	auto buffer { _device->vkDevice().createBuffer(ci) };

	/**
	 * Store page meta data and allocate
	 */

	// Warning: The following memory management only works as long as the requested data size is less or equal to the memory requirements alignment
	const auto req { _device->vkDevice().getBufferMemoryRequirements(buffer) };
	const memory::MemoryLayout layout {
		req.alignment,
		MemoryProperty::eDeviceLocal,
		req.memoryTypeBits
	};

	/**
	 * Get suitable virtual memory instance
	 */
	auto memory {
		make_uptr<VirtualMemory>(
			_device->allocator(),
			layout,
			req.size
		)
	};

	/**
	 * Gather virtual buffer instance
	 */
	auto vb {
		make_uptr<SparseBuffer>(
			std::move(memory),
			allocation_.size,
			buffer,
			ci.usage
		)
	};

	/*
	u64 offset { 0uLL };
	s64 leftPageSize { static_cast<s64>(req.alignment) };
	for (const auto pageSize : pageSizes_) {

	    if ((leftPageSize -= pageSize) > 0) {
	        continue;
	    }

	    // TBD:
		::hg::breakpoint();
	}
	 */

	const auto required {
		(allocation_.size / req.alignment) +
		((allocation_.size % req.alignment) ? 1uLL : 0uLL)
	};

	for (u64 page { 0uLL }; page < required; ++page) {
		auto vp { vb->addPage(req.alignment, req.alignment * page) };

		#ifdef _DEBUG
        assert(vp->load());
		#else
		vp->load();
		#endif
	}

	/**
	 * Bind buffer and allocated memory
	 */
	vb->updateBindingData();

	#pragma warning(push)
	#pragma warning(disable : 4996)
	vb->enqueueBindingSync(_device->graphicsQueue());
	#pragma warning(pop)

	/**
	 * Store and return virtual buffer
	 */
	auto* ptr { vb.get() };
	_indexBuffers.push_back(std::move(vb));

	//return ptr->makeView(/*offset*/0uLL, /*size*/ptr->memory()->size());
	return ptr->makeView(/*offset*/0uLL, /*size*/allocation_.size);
}

uptr<engine::gfx::SparseBufferView> GlobalResourcePool::allocateVertexBuffer(
	mref<VertexBufferAllocation> allocation_
) {

	assert(allocation_.size > 0uLL);
	auto result = _globalVertexBuffer.acquire(allocation_.size);
	if (result != nullptr) {
		return result;
	}

	::hg::breakpoint();

	/**/

	/**
	 * Create sparse vulkan buffer
	 */
	vk::BufferCreateInfo ci {
		vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency,
		allocation_.size,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::SharingMode::eExclusive,
		0uL,
		nullptr,
		nullptr
	};

	auto buffer { _device->vkDevice().createBuffer(ci) };

	/**
	 * Store page meta data and allocate
	 */

	// Warning: The following memory management only works as long as the requested data size is less or equal to the memory requirements alignment
	const auto req { _device->vkDevice().getBufferMemoryRequirements(buffer) };
	const memory::MemoryLayout layout {
		req.alignment,
		MemoryProperty::eDeviceLocal,
		req.memoryTypeBits
	};

	/**
	 * Get suitable virtual memory instance
	 */
	auto memory {
		make_uptr<VirtualMemory>(
			_device->allocator(),
			layout,
			req.size
		)
	};

	/**
	 * Gather virtual buffer instance
	 */
	auto vb {
		make_uptr<SparseBuffer>(
			std::move(memory),
			allocation_.size,
			buffer,
			ci.usage
		)
	};

	/*
	u64 offset { 0uLL };
	s64 leftPageSize { static_cast<s64>(req.alignment) };
	for (const auto pageSize : pageSizes_) {

	    if ((leftPageSize -= pageSize) > 0) {
	        continue;
	    }

	    // TBD:
		::hg::breakpoint();
	}
	 */

	const auto required {
		(allocation_.size / req.alignment) +
		((allocation_.size % req.alignment) ? 1uLL : 0uLL)
	};

	for (u64 page { 0uLL }; page < required; ++page) {
		auto vp { vb->addPage(req.alignment, req.alignment * page) };

		#ifdef _DEBUG
        assert(vp->load());
		#else
		vp->load();
		#endif
	}

	/**
	 * Bind buffer and allocated memory
	 */
	vb->updateBindingData();

	#pragma warning(push)
	#pragma warning(disable : 4996)
	vb->enqueueBindingSync(_device->graphicsQueue());
	#pragma warning(pop)

	/**
	 * Store and return virtual buffer
	 */
	auto* ptr { vb.get() };
	_vertexBuffers.push_back(std::move(vb));

	//return ptr->makeView(/*offset*/0uLL, /*size*/ptr->memory()->size());
	return ptr->makeView(/*offset*/0uLL, /*size*/allocation_.size);
}

uptr<engine::gfx::SparseTextureView> GlobalResourcePool::allocateSparseTexture(
	mref<SparseTextureAllocation> allocation_
) {
	u32 layers { 1uL };

	// Warning: Temporary Solution
	if (allocation_.type == TextureType::eCube && allocation_.extent.z == 1uL) {
		layers = 6uL;
	} else if (allocation_.extent.z != 1uL || allocation_.type != TextureType::e2d) {
		// Warning: Currently only support single layer 2d images
		::hg::breakpoint();
		return {};
	}

	assert(allocation_.extent.x <= _atlasMaxLayerExtent.x);
	assert(allocation_.extent.y <= _atlasMaxLayerExtent.y);

	/**
	 * Try find a suitable texture atlas
	 */
	ptr<SparseTexture> atlas { nullptr };

	for (const auto& candidate : _textureAtlas) {

		const auto hasMip { candidate->mipLevels() > 1uL };
		const auto isFormat { candidate->format() == allocation_.format };
		const auto isExtent {
			hasMip ?
			(allocation_.extent.x <= candidate->width() && allocation_.extent.y <= candidate->height()) :
			(allocation_.extent.x == candidate->width() && allocation_.extent.y == candidate->height())
		};

		if (isFormat && isExtent) {
			atlas = candidate.get();
		}
	}

	/**
	 *
	 */
	// TODO:
	atlas = nullptr;
	if (atlas == nullptr) {

		/**/
		TextureType payloadType { allocation_.type == TextureType::eCube ? TextureType::eCube : TextureType::e2dArray };
		if (allocation_.format == TextureFormat::eR8G8B8A8Srgb) {
			payloadType = allocation_.type;
		}
		/**/

		atlas = TextureFactory::get()->buildVirtual(
			{
				layers/* TODO: Layers */,
				allocation_.extent,
				allocation_.format,
				allocation_.mipLevels,
				payloadType,
				vk::ImageAspectFlagBits::eColor,
				vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal,
				vk::SharingMode::eExclusive
			}
		);

		auto uatlas { std::unique_ptr<SparseTexture>(atlas) };
		_textureAtlas.push_back(std::move(uatlas));
	}

	/**/

	auto view = atlas->makeView({ 0uL, layers - 1uL }, allocation_.mipLevels);

	// Attention: This is just a workaround to prevent unfitting view bindings. We need to checkout a better solution...
	if (allocation_.type == TextureType::e2d && atlas->type() == TextureType::e2dArray) {
		TextureFactory::get()->buildView(*view, { .type = TextureType::e2d });
	}

	return view;
}
