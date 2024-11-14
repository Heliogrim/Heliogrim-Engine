#include "Bmp.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/BufferFactory.hpp>
#include <Engine.GFX/Device/Device.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.GFX/Texture/SparseTexturePage.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <stb/stb_image.h>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

/**/

static engine::gfx::Buffer createStageBuffer(cref<sptr<engine::gfx::Device>> device_, u32 width_, u32 height_);

/**/

EditorBmpTextureTransformer::EditorBmpTextureTransformer(ref<engine::gfx::pool::GlobalResourcePool> pool_) noexcept :
	EditorTextureSubTransformer(),
	_pool(std::addressof(pool_)) {}

EditorBmpTextureTransformer::~EditorBmpTextureTransformer() noexcept = default;

bool EditorBmpTextureTransformer::canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {

	const auto registry = engine::Engine::getEngine()->getAssets()->getRegistry();
	const auto asset = registry->findAssetByGuid(asset_->baseImage());

	if (asset == nullptr) {
		return false;
	}

	/**/

	const auto* const image = Cast<engine::assets::Image>(asset.get());
	if (image == nullptr) {
		return false;
	}

	/**/

	for (const auto& source : image->sources()) {

		// /^(.+)\.(bmp)$/gi
		if (source.path().name().ends_with(".bmp")) {
			return true;
		}
	}

	/**/

	return false;
}

EditorBmpTextureTransformer::loader_traits::response::type EditorBmpTextureTransformer::operator()(
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

	const auto ioResult = src.first.query(
		::hg::move(src.second),
		[](_In_ cref<engine::resource::Blob> blob_) {

			auto srcSize = blob_.size();
			::hg::assertrt(srcSize > 0);

			Vector<_::byte> chunk {};
			chunk.resize(blob_.size(), _::byte {});

			std::ignore = blob_.read(streamoff {}, std::span { chunk.data(), chunk.size() });
			return chunk;
		}
	);

	::hg::assertrt(ioResult.has_value());
	auto chunk = ::hg::move(ioResult).value();

	/**/

	s32 width = -1;
	s32 height = -1;
	s32 channels = -1;
	stbi_info_from_memory(std::bit_cast<stbi_uc*>(chunk.data()), chunk.size() * sizeof(u8), &width, &height, &channels);

	::hg::assertrt(width > 0 && height > 0 && (channels == 3 || channels == 4));

	/**
	 * Store decoded data to transfer buffer
	 */

	::hg::assertrt(
		(view->type() == engine::gfx::TextureType::e2d) ||
		(view->type() == engine::gfx::TextureType::e2dArray && view->layers() == 1uL)
	);
	::hg::assertrt(view->format() == engine::gfx::TextureFormat::eR8G8B8A8Srgb);

	::hg::assertrt(view->layers() == 1uL);
	::hg::assertrt(view->mipLevels() <= 1uL);
	::hg::assertrt(view->width() == width && view->height() == height);

	const auto device = _pool->device();
	engine::gfx::Buffer stageBuffer = createStageBuffer(device, width, height);

	/**/

	constexpr u32 desiredChannels = 4uL;
	auto decoded = uptr<stbi_uc, hg::unary_fnc<void, ptr<void>>> {
		stbi_load_from_memory(
			std::bit_cast<stbi_uc*>(chunk.data()),
			chunk.size(),
			&width,
			&height,
			&channels,
			desiredChannels
		),
		stbi_image_free
	};

	::hg::assertrt(decoded != nullptr);

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
	view->owner()->enqueueBindingSync(device->graphicsQueue());

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

EditorBmpTextureTransformer::loader_traits::stream_response::type EditorBmpTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}

/**/

engine::gfx::Buffer createStageBuffer(cref<sptr<engine::gfx::Device>> device_, const u32 width_, const u32 height_) {

	using namespace ::hg::engine::gfx;

	constexpr auto format = TextureFormat::eR8G8B8A8Srgb;
	const auto formatSize = formatDataSize(format);

	const size_t dataSize = width_ * height_ * formatSize;

	/**/

	const auto factory = BufferFactory::get();
	auto stage = factory->build(
		{
			dataSize, alignof(u8),
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
