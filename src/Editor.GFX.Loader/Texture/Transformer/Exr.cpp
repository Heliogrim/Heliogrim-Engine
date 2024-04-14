#include "Exr.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/BufferFactory.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.GFX/Texture/SparseTexturePage.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>
#include <tinyexr/tinyexr.h>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

/**/

static engine::gfx::TextureFormat mapTextureFormat(cref<EXRHeader> header_) noexcept;

static engine::gfx::Buffer createStageBuffer(
	cref<sptr<engine::gfx::Device>> device_,
	engine::gfx::TextureFormat format_,
	u32 width_,
	u32 height_
);

/**/

EditorExrTextureTransformer::EditorExrTextureTransformer(ref<engine::gfx::pool::GlobalResourcePool> pool_) noexcept :
	EditorTextureSubTransformer(),
	_pool(std::addressof(pool_)) {}

EditorExrTextureTransformer::~EditorExrTextureTransformer() noexcept = default;

bool EditorExrTextureTransformer::canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {

	const auto* const registry = engine::Engine::getEngine()->getAssets()->getRegistry();
	const auto* const asset = registry->findAssetByGuid(asset_->baseImage());

	if (asset == nullptr) {
		return false;
	}

	/**/

	const auto* const image = Cast<engine::assets::Image>(asset);
	if (image == nullptr) {
		return false;
	}

	/**/

	for (const auto& source : image->sources()) {

		// /^(.+)\.(exr)$/gi
		if (source.path().name().ends_with(".exr")) {
			return true;
		}
	}

	/**/

	return false;
}

EditorExrTextureTransformer::loader_traits::response::type EditorExrTextureTransformer::operator()(
	mref<loader_traits::request::type> request_,
	mref<loader_traits::request::options> options_,
	cref<next_type> next_
) const {

	::hg::assertrt(not(options_.dataFlag & engine::gfx::loader::TextureLoadDataFlagBits::eLazyDataLoading));

	const auto* const asset = static_cast<ptr<const engine::assets::TextureAsset>>(request_);
	auto src = next_(asset, next_type::next_request_type::options {});

	/**
	 * Prepare target texture
	 */

	auto view = _pool->allocateSparseTexture(
		engine::gfx::pool::SparseTextureAllocation {
			.layers = 1uL,
			.extent = asset->getExtent(),
			.format = asset->getTextureFormat(),
			.mipLevels = { 0uL, /* asset->getMipLevelCount() - 1uL */ 0uL },
			.type = asset->getTextureType()
		}
	);

	/**
	 * Read meta data and acquire buffers
	 */

	streamsize bytes {};
	Vector<u8> chunk {};
	chunk.resize(src->size());

	std::ignore = src->get(0LL, chunk.size(), chunk.data(), bytes);

	/**/

	EXRVersion version {};
	::hg::assertrt(not ParseEXRVersionFromMemory(&version, chunk.data(), chunk.size() * sizeof(u8)));
	::hg::assertrt(not version.multipart /* Warning: No multipart / layered support for now. */);

	auto header = uptr<EXRHeader, hg::unary_fnc<int, ptr<EXRHeader>>> {
		[] {
			auto* iifeHeader = new EXRHeader {};
			InitEXRHeader(iifeHeader);
			return iifeHeader;
		}(),
		FreeEXRHeader
	};

	ptr<const char> exrErr = nullptr;
	::hg::assertrt(
		not ParseEXRHeaderFromMemory(
			header.get(),
			&version,
			chunk.data(),
			chunk.size() * sizeof(u8),
			&exrErr
		)
	);

	::hg::assertrt(mapTextureFormat(*header) != engine::gfx::TextureFormat::eUndefined);
	::hg::assertrt(view->format() == mapTextureFormat(*header));

	/**
	 * Store decoded data to transfer buffer
	 */

	::hg::assertrt(
		(view->type() == engine::gfx::TextureType::e2d) ||
		(view->type() == engine::gfx::TextureType::e2dArray && view->layers() == 1uL)
	);

	::hg::assertrt(view->layers() == 1uL);
	::hg::assertrt(view->mipLevels() <= 1uL);

	constexpr u32 desiredChannels = 4uL;
	s32 width = -1L;
	s32 height = -1L;

	auto decoded = uptr<f32, hg::unary_fnc<void, ptr<void>>> {
		[](cref<Vector<u8>> data_, ref<s32> width_, ref<s32> height_) {

			ptr<f32> iifeDecoded = nullptr;
			ptr<const char> iifeExrErr = nullptr;

			::hg::assertrt(
				LoadEXRFromMemory(
					&iifeDecoded,
					&width_,
					&height_,
					data_.data(),
					data_.size() * sizeof(u8),
					&iifeExrErr
				) == TINYEXR_SUCCESS
			);

			return iifeDecoded;
		}(chunk, width, height),
		free
	};

	::hg::assertrt(decoded != nullptr);
	::hg::assertrt(view->width() == width && view->height() == height);

	const auto device = _pool->device();
	engine::gfx::Buffer stageBuffer = createStageBuffer(device, mapTextureFormat(*header), width, height);

	/**
	 * Store image data to buffer
	 */

	stageBuffer.mapAligned();
	stageBuffer.write<_::byte>(decoded.get(), width * height * desiredChannels);
	stageBuffer.unmap();

	/**
	 * Prepare image data transfer to sparse texture
	 */

	constexpr u64 offset = 0LL;
	constexpr u32 baseLevel = 0uL;
	constexpr u32 effectedLevels = 1uL;
	constexpr u32 baseLayer = 0uL;
	constexpr u32 effectedLayers = 1uL;

	// Warning: We only support one single region at the moment
	Vector<vk::BufferImageCopy> regions {};
	regions.emplace_back(
		offset,
		0uL,
		0uL,
		vk::ImageSubresourceLayers {
			vk::ImageAspectFlagBits::eColor, baseLevel, view->baseLayer() + baseLayer, effectedLayers
		},
		vk::Offset3D {},
		vk::Extent3D { static_cast<u32>(width), static_cast<u32>(height), 1uL }
	);

	/**/

	// Warning: Temporary
	for (const auto& page : view->pages()) {
		if (page->memory()->state() != engine::gfx::VirtualMemoryPageState::eLoaded) {
			view->owner()->load(page);
		}
	}

	// Warning: Unsafe force update virtual bindings
	view->owner()->updateBindingData();
	#pragma warning(push)
	#pragma warning(disable: 4996)
	view->owner()->enqueueBindingSync(device->graphicsQueue());
	#pragma warning(pop)

	/**
	 * Transform and transfer data
	 */
	auto* const pool = device->transferQueue()->pool();
	pool->lck().acquire();

	auto cmd = pool->make();
	cmd.begin();

	vk::ImageMemoryBarrier before {
		vk::AccessFlags(), vk::AccessFlags(),
		vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		view->owner()->vkImage(),
		{ vk::ImageAspectFlagBits::eColor, baseLevel, effectedLevels, view->baseLayer() + baseLayer, effectedLayers }
	};
	vk::ImageMemoryBarrier after {
		vk::AccessFlags(), vk::AccessFlags(),
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		view->owner()->vkImage(),
		{ vk::ImageAspectFlagBits::eColor, baseLevel, effectedLevels, view->baseLayer() + baseLayer, effectedLayers }
	};

	/* Copy data from buffer to image */

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags {},
		0uL,
		nullptr,
		0uL,
		nullptr,
		1uL,
		&before
	);

	cmd.vkCommandBuffer().copyBufferToImage(
		stageBuffer.buffer,
		view->owner()->vkImage(),
		vk::ImageLayout::eTransferDstOptimal,
		static_cast<u32>(regions.size()),
		regions.data()
	);

	/* Restore layout */

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eAllGraphics,
		vk::DependencyFlags {},
		0uL,
		nullptr,
		0uL,
		nullptr,
		1uL,
		&after
	);

	/* Enqueue */

	cmd.end();
	cmd.submitWait();
	cmd.release();

	pool->lck().release();

	/**
	 * Cleanup
	 */
	stageBuffer.destroy();

	/**/

	using derived_type = engine::resource::UniqueResource<engine::gfx::TextureResource::value_type>;
	auto dst = make_smr<engine::gfx::TextureResource, derived_type>(
		new derived_type(mpt<engine::gfx::TextureLikeObject>(view.release()))
	);
	dst->setAssociation(asset);

	/**/

	return dst;
}

EditorExrTextureTransformer::loader_traits::stream_response::type EditorExrTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}

/**/

engine::gfx::TextureFormat mapTextureFormat(cref<EXRHeader> header_) noexcept {

	switch (header_.num_channels) {
		case 1: {
			return engine::gfx::TextureFormat::eR32Sfloat;
		}
		case 2: {
			return engine::gfx::TextureFormat::eR32G32Sfloat;
		}
		case 3: {
			return engine::gfx::TextureFormat::eR32G32B32Sfloat;
		}
		case 4: {
			return engine::gfx::TextureFormat::eR32G32B32A32Sfloat;
		}
		default: {
			return engine::gfx::TextureFormat::eUndefined;
		}
	}
}

engine::gfx::Buffer createStageBuffer(
	cref<sptr<engine::gfx::Device>> device_,
	const engine::gfx::TextureFormat format_,
	const u32 width_,
	const u32 height_
) {

	using namespace ::hg::engine::gfx;

	const auto formatSize = formatDataSize(format_);
	const size_t dataSize = width_ * height_ * formatSize;

	/**/

	const auto factory = BufferFactory::get();
	auto stage = factory->build(
		{
			dataSize, alignof(f32),
			MemoryProperty::eHostVisible,
			{},
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::SharingMode::eExclusive
		}
	);

	/**/

	::hg::assertrt(stage.buffer && stage.memory);
	return stage;
}
