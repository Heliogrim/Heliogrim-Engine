#include "Png.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Buffer/BufferFactory.hpp>
#include <Engine.GFX/Pool/GlobalResourcePool.hpp>
#include <Engine.GFX/Texture/SparseTexturePage.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.GFX/Texture/TextureLikeObject.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>

#define SPNG_STATIC TRUE
#include <spng/spng.h>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

/**/

static uptr<spng_ctx, unary_fnc<void, ptr<spng_ctx>>> create_png_ctx();

static void destroy_spng_ctx(ptr<spng_ctx> obj_);

static engine::gfx::Buffer createStageBuffer(cref<sptr<engine::gfx::Device>> device_, cref<spng_ihdr> ihdr_);

/**/

EditorPngTextureTransformer::EditorPngTextureTransformer(ref<engine::gfx::pool::GlobalResourcePool> pool_) noexcept :
	EditorTextureSubTransformer(),
	_pool(std::addressof(pool_)) {}

EditorPngTextureTransformer::~EditorPngTextureTransformer() noexcept = default;

bool EditorPngTextureTransformer::canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {

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
		if (source.path().name().ends_with(".png")) {
			return true;
		}
	}

	/**/

	return false;
}

EditorPngTextureTransformer::loader_traits::response::type EditorPngTextureTransformer::operator()(
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

	uptr<spng_ctx, unary_fnc<void, ptr<spng_ctx>>> decodeCtx = create_png_ctx();
	::hg::assertrt(not spng_set_png_buffer(decodeCtx.get(), chunk.data(), chunk.size() * sizeof(u8)));

	spng_ihdr ihdr;
	::hg::assertrt(not spng_get_ihdr(decodeCtx.get(), &ihdr));

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
	::hg::assertrt(view->width() == ihdr.width && view->height() == ihdr.height);

	const auto device = _pool->device();
	engine::gfx::Buffer stageBuffer = createStageBuffer(device, ihdr);

	/**
	 * Read image data from source
	 */

	u64 imageByteSize = ~0uLL;
	::hg::assertrt(not spng_decoded_image_size(decodeCtx.get(), SPNG_FMT_RGBA8, &imageByteSize));
	::hg::assertrt(stageBuffer.size >= imageByteSize);

	::hg::assertrt(not spng_decode_image(decodeCtx.get(), nullptr, 0uLL, SPNG_FMT_RGBA8, SPNG_DECODE_PROGRESSIVE));

	/**/

	const auto imageRowByteSize = imageByteSize / ihdr.height;
	spng_row_info rowInfo = { 0 };
	decltype(spng_get_row_info(decodeCtx.get(), &rowInfo)) status = 0;

	stageBuffer.mapAligned();
	Vector<_::byte> test {};
	test.resize(imageByteSize);

	do {
		status = spng_get_row_info(decodeCtx.get(), &rowInfo);
		status = status != SPNG_OK ?
			status :
			spng_decode_row(
				decodeCtx.get(),
				static_cast<ptr<_::byte>>(stageBuffer.memory->mapping) + rowInfo.row_num * imageRowByteSize,
				imageRowByteSize
			);
	} while (status == SPNG_OK && rowInfo.row_num < ihdr.height - 1);

	stageBuffer.unmap();
	::hg::assertrt(status == SPNG_OK || status == SPNG_EOI);

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
		vk::Extent3D { ihdr.width, ihdr.height, 1uL }
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

EditorPngTextureTransformer::loader_traits::stream_response::type EditorPngTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}

/**/

uptr<spng_ctx, unary_fnc<void, ptr<spng_ctx>>> create_png_ctx() {

	constexpr auto limit = 1024uLL * 1024uLL * 64uLL;

	auto obj = uptr<spng_ctx, unary_fnc<void, ptr<spng_ctx>>> {
		spng_ctx_new(0), &destroy_spng_ctx
	};

	spng_set_crc_action(obj.get(), SPNG_CRC_USE, SPNG_CRC_USE);
	spng_set_chunk_limits(obj.get(), limit, limit);

	return obj;
}

void destroy_spng_ctx(const ptr<spng_ctx> obj_) {
	spng_ctx_free(obj_);
}

engine::gfx::Buffer createStageBuffer(cref<sptr<engine::gfx::Device>> device_, cref<spng_ihdr> ihdr_) {

	using namespace ::hg::engine::gfx;

	constexpr auto format = TextureFormat::eR8G8B8A8Srgb;
	const auto formatSize = formatDataSize(format);

	const size_t dataSize = ihdr_.height * ihdr_.width * formatSize;

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
