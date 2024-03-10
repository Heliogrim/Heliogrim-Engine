#include "Ktx.hpp"

#include <filesystem>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Device/Device.hpp>
#include <Engine.GFX/Texture/SparseTexturePage.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.Logging/Logger.hpp>
#include <gli/gli.hpp>

using namespace hg::engine::gfx::loader::transformer;
using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

/**/
#pragma region KTX Format Specification
struct InternalKtxHeader {
	// Identifier
	u8 identifier[12];

	// Meta
	u32 vkFormat;
	u32 typeSize;
	u32 pixelWidth;
	u32 pixelHeight;
	u32 pixelDepth;
	u32 layerCount;
	u32 faceCount;
	u32 levelCount;
	u32 ss;

	// Indices
	u32 dfdOff;
	u32 dfdSize;
	u32 kvdOff;
	u32 kvdSize;
	u64 sgdOff;
	u64 sgdSize;
};

struct InternalKtxLevel {
	u64 byteOff;
	u64 byteSize;
	u64 ucByteSize;
};

static constexpr unsigned char ktx20Identifier[] = {
	0xAB,
	0x4B,
	0x54,
	0x58,
	0x20,
	0x32,
	0x30,
	0xBB,
	0x0D,
	0x0A,
	0x1A,
	0x0A
};
#pragma endregion
/**/

namespace hg::external::ktx {
	using extent_type = ::hg::math::uivec3;
	using offset_type = ::hg::math::ivec3;

	using layer_type = ::hg::u32;
	using level_type = ::hg::u32;

	using header_type = InternalKtxHeader;
	using level_header_type = InternalKtxLevel;

	using format_type = vk::Format;

	/**/

	enum class SuperCompressionScheme : u32 {
		eNone = 0,
		eBasisLZ = 1,
		eZStd = 2,
		eZLib = 3
	};

	struct InternalKtxKvPair {
		u32 size;
	};

	struct InternalContext {
		smr<::hg::engine::resource::Source> source;

		/**/

		header_type header;
		Vector<level_header_type> levels;
	};

	/**/

	[[nodiscard]] extent_type::value_type getWidth(cref<header_type> header_);

	[[nodiscard]] extent_type::value_type getHeight(cref<header_type> header_);

	[[nodiscard]] extent_type::value_type getDepth(cref<header_type> header_);

	[[nodiscard]] layer_type getLayerCount(cref<header_type> header_);

	[[nodiscard]] layer_type getFaceCount(cref<header_type> header_);

	[[nodiscard]] level_type getLevelCount(cref<header_type> header_);

	/**/

	[[nodiscard]] format_type getFormat(cref<header_type> header_);

	[[nodiscard]] SuperCompressionScheme getSCS(cref<header_type> header_);

	/**/

	[[nodiscard]] bool is1d(cref<header_type> header_);

	[[nodiscard]] bool is2d(cref<header_type> header_);

	[[nodisacrd]] bool is3d(cref<header_type> header_);

	[[nodiscard]] bool isCube(cref<header_type> header_);

	[[nodiscard]] bool isArray(cref<header_type> header_);

	/**/

	[[nodiscard]] u64 getDataSize(cref<InternalContext> ctx_, level_type level_);

	[[nodiscard]] extent_type calcLevelExtent(
		extent_type extent_,
		level_type level_,
		extent_type effected_ = extent_type { 1 }
	);

	[[nodiscard]] extent_type calcLevelExtent(
		extent_type extent_,
		level_type level_,
		TextureType textureType_
	);

	/**/

	[[nodiscard]] bool readHeader(_Inout_ ref<InternalContext> ctx_);

	[[nodiscard]] bool readData(cref<InternalContext> ctx_, level_type level_, _Inout_ std::span<_::byte> dst_);
}

/**/

void transformer::convertKtx(
	const non_owning_rptr<const assets::TextureAsset> asset_,
	cref<smr<resource::Source>> src_,
	const non_owning_rptr<SparseTextureView> dst_,
	cref<sptr<Device>> device_,
	const TextureLoadOptions options_
) {

	constexpr auto chunkSize = MAX(sizeof(gli::detail::FOURCC_KTX10), sizeof(ktx20Identifier));
	Vector<unsigned char> raw {};
	raw.resize(chunkSize);

	streamsize bytes {};
	src_->get(0, chunkSize, raw.data(), bytes);

	assert(bytes >= chunkSize);

	/**/

	bool isKtx20 = memcmp(raw.data(), ktx20Identifier, sizeof(ktx20Identifier)) == 0;

	if (isKtx20) {
		convertKtx20(asset_, src_, dst_, device_, options_);
		return;
	}

	/**/

	bool isKtx10 = true;
	for (size_t idx = 0; idx < sizeof(gli::detail::FOURCC_KTX10); ++idx) {
		if (raw[idx] != gli::detail::FOURCC_KTX10[idx]) {
			isKtx10 = false;
			break;
		}
	}

	if (isKtx10) {
		convertKtx10Gli(asset_, src_, dst_, device_, options_);
		return;
	}

	/**/

	#ifdef _DEBUG
	Logger::warn(
		R"(Tried to convert binary data of `{}` as KTX into texture but got no valid identifier.)",
		asset_->getAssetName()
	);
	#endif
}

void transformer::convertKtxPartial(
	const non_owning_rptr<const assets::TextureAsset> asset_,
	cref<smr<resource::Source>> src_,
	const non_owning_rptr<SparseTextureView> dst_,
	cref<sptr<Device>> device_,
	const TextureStreamOptions options_
) {

	if (options_.op == TextureStreamOp::eUnload) {

		/**
		 * Unload targeted segment and return
		 */
		unloadPartialTmp(asset_, src_, dst_, device_, options_);
		return;
	}

	/**
	 * Load the targeted segment
	 */
	convertKtx20Partial(asset_, src_, dst_, device_, options_);
}

/**/

#define IS_LOCKED_SEGMENT (false)

/**/

void deduceFromFormat(cref<gli::format> format_, ref<vk::Format> vkFormat_, ref<vk::ImageAspectFlags> aspect_) {

	switch (format_) {
		/**
		 * R Formats (2D Images)
		 *
		 * Used for alpha, roughness, displacement
		 */
		case gli::FORMAT_R16_SFLOAT_PACK16: {
			vkFormat_ = vk::Format::eR16Sfloat;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_R32_SFLOAT_PACK32: {
			vkFormat_ = vk::Format::eR32Sfloat;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}

		/**
		 * D Formats (2D Images)
		 *
		 * Used for depth images or possible for alpha blending
		 */
		case gli::FORMAT_D16_UNORM_PACK16: {
			vkFormat_ = vk::Format::eD16Unorm;
			aspect_ = vk::ImageAspectFlagBits::eDepth;
			break;
		}
		case gli::FORMAT_D32_SFLOAT_PACK32: {
			vkFormat_ = vk::Format::eD32Sfloat;
			aspect_ = vk::ImageAspectFlagBits::eDepth;
			break;
		}

		/**
		 * DS Formats (2D Images)
		 *
		 * Used for depth stencil images
		 */
		case gli::FORMAT_D16_UNORM_S8_UINT_PACK32: {
			vkFormat_ = vk::Format::eD16UnormS8Uint;
			aspect_ = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			break;
		}
		case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64: {
			vkFormat_ = vk::Format::eD32SfloatS8Uint;
			aspect_ = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			break;
		}

		/**
		 * RGB Formats (2D Images)
		 *
		 * Used for color textures like albedo, sample maps or general image
		 */
		case gli::FORMAT_RGB8_UNORM_PACK8: {
			vkFormat_ = vk::Format::eR8G8B8Unorm;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGB8_SNORM_PACK8: {
			vkFormat_ = vk::Format::eR8G8B8Snorm;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGB16_UNORM_PACK16: {
			vkFormat_ = vk::Format::eR16G16B16A16Unorm;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGB32_UINT_PACK32: {
			vkFormat_ = vk::Format::eR32G32B32Uint;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGB32_SFLOAT_PACK32: {
			vkFormat_ = vk::Format::eR32G32B32Sfloat;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}

		/**
		* RGBA Formats (2D Images)
		*
		* Used for color textures with alpha like albedo + blending or sample maps
		*/
		case gli::FORMAT_RGBA8_UNORM_PACK8: {
			vkFormat_ = vk::Format::eR8G8B8A8Unorm;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGBA8_SRGB_PACK8: {
			vkFormat_ = vk::Format::eR8G8B8A8Srgb;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGBA8_SNORM_PACK8: {
			vkFormat_ = vk::Format::eR8G8B8A8Snorm;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGBA16_UNORM_PACK16: {
			vkFormat_ = vk::Format::eR16G16B16A16Unorm;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGBA16_SFLOAT_PACK16: {
			vkFormat_ = vk::Format::eR16G16B16A16Sfloat;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGBA32_UINT_PACK32: {
			vkFormat_ = vk::Format::eR32G32B32A32Uint;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGBA32_SFLOAT_PACK32: {
			vkFormat_ = vk::Format::eR32G32B32A32Sfloat;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}

		/**
		* RGB(A) Formats (Compressed Cube Images)
		*/
		case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16: {
			vkFormat_ = vk::Format::eAstc8x8UnormBlock;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8: {
			vkFormat_ = vk::Format::eEtc2R8G8B8UnormBlock;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16: {
			// TODO: Temporary
			vkFormat_ = vk::Format::eBc2UnormBlock;
			aspect_ = vk::ImageAspectFlagBits::eColor;
			break;
		}
		default: {
			throw std::exception("Unresolved texture format.");
		}
	}
}

void deduceFromFormat(cref<vk::Format> format_, ref<vk::Format> vkFormat_, ref<vk::ImageAspectFlags> aspect_) {

	auto emberFormat { api::vkTranslateFormat(format_) };

	if (isDepthStencilFormat(emberFormat)) {
		aspect_ = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	} else if (isDepthFormat(emberFormat)) {
		aspect_ = vk::ImageAspectFlagBits::eDepth;
	} else if (isStencilFormat(emberFormat)) {
		aspect_ = vk::ImageAspectFlagBits::eStencil;
	} else {
		aspect_ = vk::ImageAspectFlagBits::eColor;
	}

	vkFormat_ = format_;
}

static [[nodiscard]] s32 calcMipDiff(cref<math::uivec3> left_, cref<math::uivec3> right_);

static Buffer createStageBuffer(cref<hg::external::ktx::InternalContext> ctx_, cref<sptr<Device>> device_) {

	using namespace ::hg::external;

	Buffer stage {};

	u64 totalSize = 0;
	for (u32 level = 0; level < ktx::getLevelCount(ctx_.header); ++level) {
		totalSize += ktx::getDataSize(ctx_, level);
	}

	/**
	 * Setup vulkan stage buffer to eager load texture
	 */
	vk::BufferCreateInfo bci {
		vk::BufferCreateFlags(),
		MAX(totalSize, 128uLL),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::SharingMode::eExclusive,
		0,
		nullptr
	};

	stage.buffer = device_->vkDevice().createBuffer(bci);
	stage.device = device_->vkDevice();

	const auto allocResult {
		memory::allocate(
			device_->allocator(),
			device_,
			stage.buffer,
			MemoryProperties { MemoryProperty::eHostVisible },
			stage.memory
		)
	};
	assert(stage.buffer);
	assert(stage.memory);

	stage.size = stage.memory->size;
	stage.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;

	/**
	 *
	 */
	stage.bind();
	stage.mapAligned();
	assert(stage.memory->mapping);

	/**
	 * Copy Data
	 */
	ptrdiff_t offset = 0;
	const ptr<_::byte> cursor = reinterpret_cast<ptr<_::byte>>(stage.memory->mapping);

	for (u32 level = 0; level < ktx::getLevelCount(ctx_.header); ++level) {

		const auto levelSize = ktx::getDataSize(ctx_, level);
		if (ktx::readData(ctx_, level, std::span { cursor + offset, levelSize })) {
			offset += levelSize;
		}
	}

	/**
	 *
	 */
	stage.flushAligned(totalSize);
	stage.unmap();

	return stage;
}

static Buffer createStageBuffer(cref<sptr<Device>> device_, const u64 byteSize_) {

	using namespace ::hg::external;

	Buffer stage {};

	/**
	 * Setup vulkan stage buffer to eager load texture
	 */
	vk::BufferCreateInfo bci {
		vk::BufferCreateFlags(),
		MAX(byteSize_, 128uLL),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::SharingMode::eExclusive,
		0,
		nullptr
	};

	stage.buffer = device_->vkDevice().createBuffer(bci);
	stage.device = device_->vkDevice();

	const auto allocResult {
		memory::allocate(
			device_->allocator(),
			device_,
			stage.buffer,
			MemoryProperties { MemoryProperty::eHostVisible },
			stage.memory
		)
	};
	assert(stage.buffer);
	assert(stage.memory);

	stage.size = stage.memory->size;
	stage.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;

	/**
	 *
	 */
	stage.bind();

	return stage;
}

/**/

void transformer::convertKtx10Gli(
	const non_owning_rptr<const engine::assets::TextureAsset> asset_,
	cref<smr<engine::resource::Source>> src_,
	const non_owning_rptr<SparseTextureView> dst_,
	cref<sptr<Device>> device_,
	const TextureLoadOptions options_
) {

	const auto srcSize = src_->size();
	assert(srcSize > 0);

	Vector<char> tmp {};
	tmp.resize(srcSize);

	streamsize bytes {};
	src_->get(0, srcSize, tmp.data(), bytes);

	/**/

	assert(bytes >= srcSize);
	gli::texture glitex = gli::load_ktx(tmp.data(), bytes);

	/**/

	const auto lvlZeroExt = glitex.extent(0);
	math::ivec3 extent = {
		lvlZeroExt.x,
		lvlZeroExt.y,
		lvlZeroExt.z
	};

	const auto lvlZeroForm = glitex.format();
	auto format = vk::Format::eUndefined;
	auto aspect = vk::ImageAspectFlags {};
	auto create = vk::ImageCreateFlags();
	vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc |
		vk::ImageUsageFlagBits::eTransferDst |
		vk::ImageUsageFlagBits::eSampled;

	deduceFromFormat(lvlZeroForm, format, aspect);

	const auto faceCount = glitex.faces();
	if (faceCount == 6) {
		create = vk::ImageCreateFlagBits::eCubeCompatible;
	}

	/* Validate Matching Descriptor */
	#ifdef _DEBUG

	if (dst_->format() != api::vkTranslateFormat(format)) {
		__debugbreak();
	}

	if (dst_->mipLevels() < glitex.levels()) {
		__debugbreak();
	}

	if (create & vk::ImageCreateFlagBits::eCubeCompatible && dst_->type() != TextureType::eCube) {
		__debugbreak();
	}

	// Validate Aspect Flag Bits
	// Validate Usage Flag Bits

	#endif

	/**
	 * Staging Buffer
	 */
	Buffer stage {};

	vk::BufferCreateInfo bci {
		vk::BufferCreateFlags(),
		glitex.size(),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::SharingMode::eExclusive,
		0,
		nullptr
	};

	stage.buffer = device_->vkDevice().createBuffer(bci);
	stage.device = device_->vkDevice();

	const auto allocResult {
		memory::allocate(
			device_->allocator(),
			device_,
			stage.buffer,
			MemoryProperties { MemoryProperty::eHostVisible } | MemoryProperty::eHostCoherent,
			stage.memory
		)
	};
	assert(stage.buffer);
	assert(stage.memory);

	stage.size = stage.memory->size;
	stage.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;

	/**
	 *
	 */
	stage.bind();
	stage.map();
	assert(stage.memory->mapping);

	/**
	 * Copy Data
	 */
	stage.write(glitex.data(), glitex.size());

	/**
	 *
	 */
	stage.unmap();

	/**
	 * Fetch Region per Layer
	 */

	const auto minMipLevel = std::max(dst_->minMipLevel(), static_cast<u32>(glitex.base_level()));
	const auto maxMipLevel = std::min(dst_->maxMipLevel(), static_cast<u32>(glitex.max_level()));

	/**/

	// Prevent signed to unsigned wrapping (-1 ~> ~0)
	assert(maxMipLevel <= dst_->maxMipLevel());
	assert(minMipLevel <= maxMipLevel);

	/**/

	Vector<vk::BufferImageCopy> regions {};
	uint32_t offset = 0;

	if (create & vk::ImageCreateFlagBits::eCubeCompatible) {

		const gli::texture_cube ct { glitex };

		assert(dst_->depth() == 1uL);

		for (uint32_t face = 0; face < glitex.faces(); face++) {
			for (uint32_t level = minMipLevel; level <= maxMipLevel; ++level) {

				const auto se = ct[face][level].extent();

				vk::BufferImageCopy copy {
					offset,
					0,
					0,
					{
						aspect,
						level,
						face,
						1uL
					},
					vk::Offset3D(),
					vk::Extent3D {
						static_cast<uint32_t>(se.x),
						static_cast<uint32_t>(se.y),
						1uL
					}
				};

				regions.push_back(copy);
				offset += static_cast<uint32_t>(ct[face][level].size());
			}
		}

	} else {
		/**
		 * Copy Buffer Image (2D / 2D Array)
		 */
		for (uint32_t level = minMipLevel; level <= maxMipLevel; ++level) {
			vk::BufferImageCopy copy {
				offset,
				0,
				0,
				{
					aspect,
					level,
					0,
					dst_->type() == TextureType::e2dArray ? dst_->depth() : 1
				},
				vk::Offset3D(),
				vk::Extent3D {
					dst_->width() / (0x1 << level),
					dst_->height() / (0x1 << level),
					dst_->type() == TextureType::e2dArray ? 1 : dst_->depth()
				}
			};

			regions.push_back(copy);
			offset += static_cast<uint32_t>(glitex.size(level));
		}
	}

	/**
	 * Ensure virtual memory is residential
	 */
	for (const auto& page : dst_->pages()) {

		if (IS_LOCKED_SEGMENT) {
			// TODO: Lock effected memory and texture pages!
		}

		if (page->memory()->state() != VirtualMemoryPageState::eLoaded) {
			dst_->owner()->load(page);
		}

	}

	/**
	* Update virtual binding data
	*/
	auto tex { dst_->owner() };
	const_cast<SparseTexture*>(tex.get())->updateBindingData();
	#pragma warning(push)
	#pragma warning(disable: 4996)
	const_cast<SparseTexture*>(tex.get())->enqueueBindingSync(device_->graphicsQueue());
	#pragma warning(pop)

	/**
	 * Capture commands to copy data to image
	 */
	auto* const pool = device_->transferQueue()->pool();
	pool->lck().acquire();
	CommandBuffer cmd = pool->make();
	cmd.begin();

	/**
	 * Pre-Transform image layout
	 */

	vk::ImageMemoryBarrier simb {
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		dst_->owner()->vkImage(),
		{
			aspect,
			0,
			(maxMipLevel - minMipLevel) + 1,
			dst_->baseLayer(),
			dst_->layers()
		}
	};

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		0,
		nullptr,
		0,
		nullptr,
		1,
		&simb
	);

	/**
	 * Copy data from buffer to image
	 */

	cmd.vkCommandBuffer().copyBufferToImage(
		stage.buffer,
		dst_->owner()->vkImage(),
		vk::ImageLayout::eTransferDstOptimal,
		static_cast<uint32_t>(regions.size()),
		regions.data()
	);

	/**
	 * Restore Layout
	 */
	vk::ImageMemoryBarrier eimb {
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		dst_->owner()->vkImage(),
		{
			aspect,
			0,
			(maxMipLevel - minMipLevel) + 1,
			dst_->baseLayer(),
			dst_->layers()
		}
	};

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eAllCommands,
		vk::DependencyFlags(),
		0,
		nullptr,
		0,
		nullptr,
		1,
		&eimb
	);

	/**
	 * Dispatch commands
	 */

	cmd.end();
	cmd.submitWait();
	cmd.release();

	pool->lck().release();

	/**
	 * Cleanup
	 */
	stage.destroy();
	glitex.clear();

	tmp.clear();
}

void transformer::convertKtx20(
	const non_owning_rptr<const engine::assets::TextureAsset> asset_,
	cref<smr<engine::resource::Source>> src_,
	const non_owning_rptr<SparseTextureView> dst_,
	cref<sptr<Device>> device_,
	const TextureLoadOptions options_
) {

	if (options_.dataFlag & TextureLoadDataFlagBits::eLazyDataLoading) {

		/**
		* Transform and data transfer
		*/
		auto* const pool = device_->transferQueue()->pool();
		pool->lck().acquire();
		CommandBuffer cmd = pool->make();
		cmd.begin();

		/**/
		vk::ImageMemoryBarrier barrier {};
		/**/

		/**
		 * Copy Data to Image
		 */
		barrier = {
			vk::AccessFlags(),
			vk::AccessFlags(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			dst_->owner()->vkImage(),
			{
				vk::ImageAspectFlagBits::eColor,
				0,
				dst_->mipLevels(),
				dst_->baseLayer(),
				dst_->layers()
			}
		};

		cmd.vkCommandBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::DependencyFlags(),
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		cmd.end();
		cmd.submitWait();
		cmd.release();

		pool->lck().release();

		return;
	}

	/**/

	using namespace ::hg::external;

	ktx::InternalContext ctx {
		src_
	};

	if (not ktx::readHeader(ctx)) {
		return;
	}

	Buffer stage = createStageBuffer(ctx, device_);

	/**/

	bool validType = false;
	switch (dst_->type()) {
		case TextureType::e2d: {
			validType = ktx::is2d(ctx.header) && not ktx::isArray(ctx.header);
			break;
		}
		case TextureType::e2dArray: {
			validType = ktx::is2d(ctx.header) && ktx::isArray(ctx.header);
			validType = ktx::is2d(ctx.header);
			break;
		}
		case TextureType::e3d: {
			validType = ktx::is3d(ctx.header) && not ktx::isArray(ctx.header);
			break;
		}
		case TextureType::eCube: {
			validType = ktx::isCube(ctx.header) && not ktx::isArray(ctx.header);
			break;
		}
	}

	if (not validType) {
		stage.destroy();
	}

	/**/

	vk::Format format {};
	vk::ImageAspectFlags aspect {};
	deduceFromFormat(ktx::getFormat(ctx.header), format, aspect);

	const auto effectedLayers = MAX(ktx::getLayerCount(ctx.header), ktx::getFaceCount(ctx.header));
	const auto effectedLevels = MIN(ktx::getLevelCount(ctx.header), dst_->mipLevels());

	/**/

	u64 offset = 0;
	Vector<vk::BufferImageCopy> regions {};

	for (u32 level = 0; level < effectedLevels; ++level) {

		const math::uivec3 extent { ktx::getWidth(ctx.header), ktx::getHeight(ctx.header), ktx::getDepth(ctx.header) };
		const auto levelExtent = ktx::calcLevelExtent(extent, level);

		vk::BufferImageCopy copy {
			offset,
			0,
			0,
			{
				aspect,
				level,
				0,
				effectedLayers
			},
			vk::Offset3D(),
			vk::Extent3D { levelExtent.x, levelExtent.y, levelExtent.z }
		};

		regions.push_back(std::move(copy));
		offset += ktx::getDataSize(ctx, level);
	}

	/**/

	// Warning: Temporary
	for (const auto& page : dst_->pages()) {

		if (IS_LOCKED_SEGMENT) {
			// TODO: Lock effected memory and texture pages!!!
		}

		if (page->memory()->state() != VirtualMemoryPageState::eLoaded) {
			dst_->owner()->load(page);
		}
	}

	/**
	* Update virtual binding data
	*/
	const auto tex { dst_->owner() };
	const_cast<SparseTexture*>(tex.get())->updateBindingData();
	#pragma warning(push)
	#pragma warning(disable: 4996)
	const_cast<SparseTexture*>(tex.get())->enqueueBindingSync(device_->graphicsQueue());
	#pragma warning(pop)

	/**
	* Transform and data transfer
	*/
	auto* const pool = device_->transferQueue()->pool();
	pool->lck().acquire();
	CommandBuffer cmd = pool->make();
	cmd.begin();

	/**/
	vk::ImageMemoryBarrier preBarrier {};
	vk::ImageMemoryBarrier postBarrier {};
	/**/

	/**
	 * Copy Data to Image
	 */
	preBarrier = {
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		dst_->owner()->vkImage(),
		{
			aspect,
			0,
			effectedLevels,
			dst_->baseLayer(),
			effectedLayers
		}
	};

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		0,
		nullptr,
		0,
		nullptr,
		1,
		&preBarrier
	);

	cmd.vkCommandBuffer().copyBufferToImage(
		stage.buffer,
		dst_->owner()->vkImage(),
		vk::ImageLayout::eTransferDstOptimal,
		static_cast<u32>(regions.size()),
		regions.data()
	);

	/**
	* Restore Layout
	*/
	postBarrier = {
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		dst_->owner()->vkImage(),
		{
			aspect,
			0,
			effectedLevels,
			dst_->baseLayer(),
			effectedLayers
		}
	};

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eAllGraphics,
		vk::DependencyFlags(),
		0,
		nullptr,
		0,
		nullptr,
		1,
		&postBarrier
	);

	cmd.end();
	cmd.submitWait();
	cmd.release();

	pool->lck().release();

	/**
	* Cleanup
	*/
	stage.destroy();
}

void transformer::convertKtx20Partial(
	const non_owning_rptr<const engine::assets::TextureAsset> asset_,
	cref<smr<engine::resource::Source>> src_,
	const non_owning_rptr<SparseTextureView> dst_,
	cref<sptr<Device>> device_,
	const TextureStreamOptions options_
) {

	using namespace ::hg::external;

	/**/

	math::uivec3 reqExtent { math::compMax<u32>(options_.extent, math::uivec3 { 1 }) };
	math::uivec3 reqOffset { options_.offset };

	/**/

	const auto dstMipExtent = math::compMax<u32>(dst_->extent() >> options_.mip, math::uivec3 { 1 });

	if (reqOffset.x >= dstMipExtent.x || reqOffset.y >= dstMipExtent.y || reqOffset.z >= dstMipExtent.z) {
		return;
	}

	const math::uivec3 dstLeftExt = {
		dstMipExtent.x - reqOffset.x,
		dstMipExtent.y - reqOffset.y,
		dstMipExtent.y - reqOffset.z
	};
	const math::uivec3 corrected = math::compMax<u32>(
		dstLeftExt,
		math::uivec3 { 1 }
	);

	reqExtent = math::compMin<u32>(reqExtent, corrected);

	/**
	* Check for memory changes to prevent excessive file loading
	*/
	#pragma region Preserve Memory Changes

	/* (Optimization I :: Binary Search Begin) */
	auto pageIter { dst_->pages().begin() };

	if (options_.mip > 0) {
		// We assume that the pages of each texture have a geometric distribution, therefore we might skip the first subset
		// Warning: Only when using 8k Textures :: 8192 / 128 ~> 64 ~> 4096 Pages

		auto estSkip { 0uLL };
		if (dst_->pages().size() >= 5200uLL) {
			estSkip = 4095uLL;
		}

		const auto flbe {
			std::ranges::lower_bound(
				dst_->pages().begin() + estSkip,
				dst_->pages().end(),
				options_.mip,
				std::ranges::less {},
				[](const auto& entry_) {
					return entry_->mipLevel();
				}
			)
		};

		pageIter += std::distance(dst_->pages().begin(), flbe);
	}
	/**/

	u32 mipOff { static_cast<u32>(std::distance(dst_->pages().begin(), pageIter)) };

	bool changedMemory { false };

	if (mipOff >= dst_->pages().size()) {
		mipOff = dst_->pages().size() - 1;
	}

	if (dst_->pages()[mipOff]->flags() & SparseTexturePageFlag::eOpaqueTail) {

		changedMemory = dst_->pages()[mipOff]->memory()->state() != VirtualMemoryPageState::eLoaded;

		if (changedMemory) {
			dst_->owner()->load(dst_->pages()[mipOff]);
		}
	}

	if (options_.mip < dst_->owner()->mipTailFirstLod()) {

		//constexpr auto pageExt { 128uL };
		const math::uivec3 pageExt { dst_->owner()->granularity() };

		const math::uivec3 acc {
			math::compMax<math::uivec3::value_type>(
				(dst_->extent() >> options_.mip) / pageExt,
				math::uivec3 { 1uL }
			)
		};
		const math::uivec3 off { reqOffset / pageExt };
		const math::uivec3 ext {
			math::compMax<math::uivec3::value_type>(reqExtent / pageExt, math::uivec3 { 1uL })
		};

		/**
		 * Reexpand the ext to the requested size to apply page granularity for data loading
		 */
		reqExtent = ext * pageExt;

		/**
		 * Reexpand the off to the requested offset to apply page granularity for data loading
		 */
		reqOffset = off * pageExt;

		u32 oi { off.x + off.y * acc.x };
		const auto out { (off.x + ext.x) + (off.y + ext.y - 1) * acc.x };

		while (oi < out) {

			for (u32 ix { 0uL }; ix < ext.x; ++ix) {

				u32 idx { mipOff + oi + ix };
				auto page { dst_->pages()[idx] };

				bool effected { false };
				if (page->mipLevel() != options_.mip) {
					continue;
				}

				const auto minPage { page->offset() };
				const auto maxPage { page->offset() + page->extent() };

				const auto minDst { reqOffset };
				const auto maxDst { reqOffset + reqExtent };

				const auto pX { minPage.x >= maxDst.x || maxPage.x <= minDst.x };
				const auto pY { minPage.y >= maxDst.y || maxPage.y <= minDst.y };
				const auto pZ { minPage.z >= maxDst.z || maxPage.z <= minDst.z };

				effected = !(pX || pY || pZ);

				if (effected && page->memory()->state() != VirtualMemoryPageState::eLoaded) {
					dst_->owner()->load(page);
					changedMemory = true;
				}

			}
			oi += acc.x;

		}
	}

	#if FALSE
    /* (Optimization II :: Early-Exit Iterator Loop) */
    bool changedMemory{ false };
    for (; pageIter != dst_->pages().end() && (*pageIter)->mipLevel() == options_.mip; ++pageIter) {

        auto* page{ *pageIter };

        if (page->layer() != options_.layer) {
            continue;
        }

        // Assuming the pages are sorted by there mip and linearized as grid, we can compute some `bounding box` to check
        //bool testLower { false };
        bool testUpper{ false };

        /*
         *              |                   |
         *   Offset     |   Y-Skip (ax*dy)  |
         *              |                   |
         *  ------------|-------------------|-------
         *              |                   |
         *  X-Skip (dx) |   Extent          |   X-Break (ix >= ox+ex)
         *              |                   |
         *  ------------|-------------------|-------
         *              |                   |
         *              |   Y-Break (iy >= oy+ey) | Exit
         *              |                   |
         */

	#if FALSE
        constexpr auto pageExt{ 128uL };
        const math::uivec3 acc{ (dst_->width() >> options_.mip) / pageExt };
        const math::uivec3 off{ options_.offset / pageExt };
        const math::uivec3 ext{ options_.extent / pageExt };

        u32 oi{ off.x + off.y * acc.x };
        const auto out{ (off.x + ext.x) + (off.y + ext.y) * acc.x };

        while (oi < out) {
            for (u32 ix{ 0uL }; ix < ext.x; ++ix) {
                u32 idx{ oi + ix };
            }
            oi += acc.x;
        }
	#endif

        bool effected{ false };
        if (page->flags() & SparseTexturePageFlag::eOpaqueTail) {
            effected = page->mipLevel() <= options_.mip;

        }
        else {

            if (page->mipLevel() != options_.mip) {
                continue;
            }

            const auto minPage{ page->offset() };
            const auto maxPage{ page->offset() + page->extent() };

            const auto minDst{ options_.offset };
            const auto maxDst{ options_.offset + options_.extent };

            const auto pX{ minPage.x >= maxDst.x || maxPage.x <= minDst.x };
            const auto pY{ minPage.y >= maxDst.y || maxPage.y <= minDst.y };
            const auto pZ{ minPage.z >= maxDst.z || maxPage.z <= minDst.z };

            effected = !(pX || pY || pZ);

            // Upper `bounding box` check for early exit
            if (maxPage.x >= maxDst.x && maxPage.y >= maxDst.y && maxPage.z >= maxDst.z) {
                testUpper = true;
            }
        }

        if (effected && page->memory()->state() != VirtualMemoryPageState::eLoaded) {
            dst_->owner()->load(page);
            changedMemory = true;
        }

        // Early-Exit on last effected element
        if (testUpper) {
            break;
        }
    }
	#endif

	// TODO: !!! Move dirty flagging and update tracking into virtual texture itself !!!
	if (changedMemory) {
		auto tex { dst_->owner() };
		const_cast<SparseTexture*>(tex.get())->updateBindingData();
		#pragma warning(push)
		#pragma warning(disable: 4996)
		const_cast<SparseTexture*>(tex.get())->enqueueBindingSync(device_->transferQueue());
		#pragma warning(pop)
	}

	#pragma endregion

	/**
	 *
	 */
	if (!changedMemory && not(options_.mip >= dst_->owner()->mipTailFirstLod())) {
		return;
	}
	/**
	 *
	 */

	#pragma region KTX Header

	ktx::InternalContext ctx {
		src_
	};

	if (not ktx::readHeader(ctx)) {
		return;
	}

	// Format
	vk::Format format {};
	vk::ImageAspectFlags aspect {};
	deduceFromFormat(ktx::getFormat(ctx.header), format, aspect);

	// Meta
	const ktx::extent_type ktxExtent {
		ktx::getWidth(ctx.header), ktx::getHeight(ctx.header), ktx::getDepth(ctx.header)
	};

	#pragma endregion

	/* Check Mip Diff */

	const math::uivec3 ownerExtent {
		dst_->owner()->width(),
		dst_->owner()->height(),
		dst_->owner()->depth()
	};
	auto mipDiff = calcMipDiff(ktxExtent, ownerExtent);

	assert(static_cast<s32>(options_.mip) + mipDiff >= 0);

	/**/

	// TODO: Check why `base - option` instead of `base + layer` ?!?
	const u32 srcLayer { dst_->baseLayer() - options_.layer };
	const u32 srcMip { options_.mip + mipDiff };

	if (srcMip > ktx::getLevelCount(ctx.header)) {
		#ifdef _DEBUG
		// We can't load data into a mip level which is not present within the source data
		__debugbreak();
		#endif
		return;
	}

	/**/

	#pragma region KTX Source Data

	const ktx::extent_type levelExtent = ktx::calcLevelExtent(ktxExtent, srcMip);
	const auto& ktxLevel = ctx.levels[srcMip];

	const auto formatSize = formatDataSize(api::vkTranslateFormat(format));
	const auto blockSize = formatBlockSize(api::vkTranslateFormat(format));

	if (blockSize.x != 1 || blockSize.y != 1 || blockSize.z != 1) {
		#ifdef _DEBUG
		// We currently don't support (compressed) block encoding.
		__debugbreak();
		#endif
		return;
	}

	/* Create stage buffer */

	u64 stageSize = ~0;
	if (options_.extent.zero()) {
		stageSize = levelExtent.x * levelExtent.y * levelExtent.z * formatSize;
	} else {
		stageSize = reqExtent.x * reqExtent.y * reqExtent.z * formatSize;
	}

	#ifdef _DEBUG
	if (stageSize <= 0 || stageSize >= ~0uLL) {
		__debugbreak();
		return;
	}
	#endif

	Buffer stage = createStageBuffer(device_, stageSize);

	/* Load data to stage */

	const bool useFastCopy { options_.extent.zero() || options_.extent == levelExtent };
	if (useFastCopy) {

		if (not stage.memory->mapping) {
			stage.mapAligned();
		}

		std::span<_::byte> bufferMemory { reinterpret_cast<ptr<_::byte>>(stage.memory->mapping), stage.memory->size };
		const auto succeeded = ktx::readData(ctx, srcMip, bufferMemory);

		if (not succeeded) {
			stage.destroy();
			return;
		}

		stage.flushAligned();
		stage.unmap();

	} else {

		if (ktx::getSCS(ctx.header) != ktx::SuperCompressionScheme::eNone) {
			#ifdef _DEBUG
			// We can't use block loading with super compression
			__debugbreak();
			#endif
			return;
		}

		if (formatSize <= 0) {
			#ifdef _DEBUG
			// Missing block size is indicator for unsupported sparse loading
			// TODO: Check whether we can break up at least ASTC formats
			__debugbreak();
			#endif
			return;
		}

		// :: level
		// :: level :: layer
		// :: level :: layer :: face
		// :: level :: layer :: face :: z
		// :: level :: layer :: face :: z :: y
		// :: level :: layer :: face :: z :: y :: x

		const auto globalLevelOffset = ktxLevel.byteOff;

		const auto layerCount = ktx::getLayerCount(ctx.header);
		const auto faceCount = ktx::getFaceCount(ctx.header);

		const auto rowSize = levelExtent.x * formatSize;
		const auto planeSize = levelExtent.y * rowSize;
		const auto sliceSize = levelExtent.z * planeSize;
		const auto faceSize = sliceSize;
		const auto layerSize = faceSize * faceCount;

		const auto layerOffset = options_.layer * layerSize;

		/**/

		const auto globalOffset = globalLevelOffset + layerOffset;

		/**/

		const auto minExt { reqOffset };
		const auto maxExt { reqOffset + reqExtent };

		// the file is linearized at the x-dimension, so we can read the whole contiguous sequence
		const streamsize seqRowSize = (maxExt.x - minExt.x) * formatSize;
		const streamoff seqRowOff = minExt.x * formatSize;

		/**/

		if (not stage.memory->mapping) {
			stage.mapAligned();
		}

		const ptr<_::byte> memory = reinterpret_cast<ptr<_::byte>>(stage.memory->mapping);
		streampos memPos = 0;

		// for (u32 face { 0uL }; face < faceCount; ++face) {
		for (u32 slice { minExt.z }; slice < maxExt.z; ++slice) {

			const streamoff sliceOff = slice * planeSize;
			for (u32 row { minExt.y }; row < maxExt.y; ++row) {

				const streamoff planeOff = row * rowSize;
				streamsize bytes = -1;

				src_->get(
					globalOffset + sliceOff + planeOff + seqRowOff,
					seqRowSize,
					memory + memPos,
					bytes
				);

				memPos += seqRowSize;
			}
		}
		// }

		stage.flushAligned();
		stage.unmap();
	}

	#pragma endregion

	/**
	* Fetch Region per Layer
	*/
	Vector<vk::BufferImageCopy> regions {};

	/**
	* Copy Buffer Image (2D / 2D Array)
	*/
	regions.push_back(
		vk::BufferImageCopy {
			0uLL,
			0uL,
			0uL,
			vk::ImageSubresourceLayers {
				vk::ImageAspectFlagBits::eColor,
				options_.mip,
				options_.layer,
				1uL
			},
			vk::Offset3D(
				static_cast<s32>(reqOffset.x),
				static_cast<s32>(reqOffset.y),
				static_cast<s32>(reqOffset.z)
			),
			vk::Extent3D(
				reqExtent.x,
				reqExtent.y,
				reqExtent.z
			)
		}
	);

	#if TRUE
	if (dst_->extent().x == 8192 && dst_->format() == TextureFormat::eR8G8B8A8Unorm) {
		IM_CORE_LOGF("Loading Texture [{}, {}] at mip level `{}`.", reqOffset.x, reqOffset.y, options_.mip);
	}
	#endif

	/**
	* Copy Data to Image
	*/
	auto* const pool = device_->transferQueue()->pool();
	pool->lck().acquire();
	CommandBuffer cmd = pool->make();
	cmd.begin();

	vk::ImageMemoryBarrier simb {
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		dst_->owner()->vkImage(),
		vk::ImageSubresourceRange {
			vk::ImageAspectFlagBits::eColor,
			options_.mip,
			1uL,
			options_.layer,
			1uL
		}
	};

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		0,
		nullptr,
		0,
		nullptr,
		1,
		&simb
	);

	cmd.vkCommandBuffer().copyBufferToImage(
		stage.buffer,
		dst_->owner()->vkImage(),
		vk::ImageLayout::eTransferDstOptimal,
		static_cast<u32>(regions.size()),
		regions.data()
	);

	/**
	* Restore Layout
	*/
	vk::ImageMemoryBarrier eimb {
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		dst_->owner()->vkImage(),
		vk::ImageSubresourceRange {
			vk::ImageAspectFlagBits::eColor,
			options_.mip,
			1uL,
			options_.layer,
			1uL
		}
	};

	cmd.vkCommandBuffer().pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		0,
		nullptr,
		0,
		nullptr,
		1,
		&eimb
	);

	cmd.end();
	cmd.submitWait();
	cmd.release();

	pool->lck().release();

	/**
	* Cleanup
	*/
	stage.destroy();
}

void transformer::unloadPartialTmp(
	const non_owning_rptr<const engine::assets::TextureAsset> asset_,
	cref<smr<engine::resource::Source>> src_,
	const non_owning_rptr<SparseTextureView> dst_,
	cref<sptr<Device>> device_,
	const TextureStreamOptions options_
) {

	/* (Optimization I :: Binary Search Begin) */
	const auto flbe {
		std::ranges::lower_bound(
			dst_->pages(),
			options_.mip,
			std::ranges::less {},
			[](const auto& entry_) {
				return entry_->mipLevel();
			}
		)
	};
	/**/

	/* (Optimization II :: Early-Exit Iterator Loop) */
	bool changedMemory { false };
	for (
		auto iter { dst_->pages().begin() + std::distance(dst_->pages().begin(), flbe) };
		iter != dst_->pages().end() && (*iter)->mipLevel() == options_.mip;
		++iter
	) {

		auto page { *iter };

		if (page->layer() != options_.layer) {
			continue;
		}

		bool effected { false };
		if (page->flags() & SparseTexturePageFlag::eOpaqueTail) {
			// TODO: effected = page->mipLevel() <= options_->mip;

		} else {

			if (page->mipLevel() != options_.mip) {
				continue;
			}

			const auto minPage { page->offset() };
			const auto maxPage { page->offset() + page->extent() };

			const auto minDst { options_.offset };
			const auto maxDst { options_.offset + options_.extent };

			const auto pX { minPage.x >= maxDst.x || maxPage.x <= minDst.x };
			const auto pY { minPage.y >= maxDst.y || maxPage.y <= minDst.y };
			const auto pZ { minPage.z >= maxDst.z || maxPage.z <= minDst.z };

			effected = !(pX || pY || pZ);
		}

		if (effected && page->memory()->state() == VirtualMemoryPageState::eLoaded) {
			dst_->owner()->unload(page);
			changedMemory = true;
		}
	}

	/**
	*
	*/
	if (changedMemory) {
		auto tex { dst_->owner() };
		const_cast<SparseTexture*>(tex.get())->updateBindingData();
		#pragma warning(push)
		#pragma warning(disable: 4996)
		const_cast<SparseTexture*>(tex.get())->enqueueBindingSync(device_->transferQueue());
		#pragma warning(pop)
	}
}

/**/

#pragma region KTX Specification Implementation

math::uivec3::value_type external::ktx::getWidth(cref<header_type> header_) {
	return header_.pixelWidth;
}

math::uivec3::value_type external::ktx::getHeight(cref<header_type> header_) {
	return header_.pixelHeight;
}

math::uivec3::value_type external::ktx::getDepth(cref<header_type> header_) {
	return header_.pixelDepth;
}

external::ktx::layer_type external::ktx::getLayerCount(cref<header_type> header_) {
	return header_.layerCount;
}

external::ktx::layer_type external::ktx::getFaceCount(cref<header_type> header_) {
	return header_.faceCount;
}

external::ktx::level_type external::ktx::getLevelCount(cref<header_type> header_) {
	return header_.levelCount ? header_.levelCount : 1;
}

external::ktx::format_type external::ktx::getFormat(cref<header_type> header_) {
	return *reinterpret_cast<const vk::Format*>(&header_.vkFormat);
}

external::ktx::SuperCompressionScheme external::ktx::getSCS(cref<header_type> header_) {
	return *reinterpret_cast<const ptr<const SuperCompressionScheme>>(&header_.ss);
}

bool external::ktx::is1d(cref<header_type> header_) {

	// Required 
	if (header_.pixelWidth <= 0 || header_.faceCount != 1) {
		return false;
	}

	// Denied
	if (header_.pixelHeight > 0 || header_.pixelDepth > 0) {
		return false;
	}

	return true;
}

bool external::ktx::is2d(cref<header_type> header_) {

	// Required
	if (header_.pixelWidth <= 0 || header_.pixelHeight <= 0 || header_.faceCount != 1) {
		return false;
	}

	// Denied
	if (header_.pixelDepth > 0) {
		return false;
	}

	return true;
}

bool external::ktx::is3d(cref<header_type> header_) {

	// Required
	if (header_.pixelWidth <= 0 || header_.pixelHeight <= 0 || header_.pixelDepth <= 0 || header_.faceCount != 1) {
		return false;
	}

	return true;
}

bool external::ktx::isCube(cref<header_type> header_) {

	// Required
	if (header_.pixelWidth <= 0 || header_.pixelHeight <= 0 || header_.faceCount != 6) {
		return false;
	}

	// Denied
	if (header_.pixelDepth > 0) {
		return false;
	}

	return true;
}

bool external::ktx::isArray(cref<header_type> header_) {
	return header_.layerCount > 0;
}

u64 external::ktx::getDataSize(cref<InternalContext> ctx_, level_type level_) {

	if (level_ >= ctx_.header.levelCount) {
		return 0;
	}

	return ctx_.levels[level_].ucByteSize;
}

external::ktx::extent_type external::ktx::calcLevelExtent(
	extent_type extent_,
	level_type level_,
	extent_type effected_
) {

	auto result = extent_;
	for (; level_ > 0; --level_) {
		result = result >> 1;
	}

	if (effected_.x <= 0) {
		result.x = extent_.x;
	}

	if (effected_.y <= 0) {
		result.y = extent_.y;
	}

	if (effected_.z <= 0) {
		result.z = extent_.z;
	}

	result = math::compMax<extent_type::value_type>(result, extent_type { 1 });
	return result;
}

external::ktx::extent_type external::ktx::calcLevelExtent(
	extent_type extent_,
	level_type level_,
	TextureType textureType_
) {
	return calcLevelExtent(std::move(extent_), std::move(level_));
}

bool external::ktx::readHeader(ref<InternalContext> ctx_) {

	constexpr auto level_size = sizeof(InternalKtxLevel);
	constexpr auto header_size = sizeof(InternalKtxHeader);

	/**/

	streamsize bytes = -1;
	ctx_.source->get(0, header_size, &ctx_.header, bytes);

	if (bytes < header_size) {
		return false;
	}

	/**/

	if (memcmp(ctx_.header.identifier, ktx20Identifier, sizeof(ktx20Identifier)) != 0) {
		return false;
	}

	if (ctx_.header.faceCount != 1 && ctx_.header.faceCount != 6) {
		IM_CORE_ERROR("Tried to load ktx file with invalid face count. ( Spec.: 1 or 6 )");
		return false;
	}

	/**/

	if (ctx_.header.sgdSize > 0) {
		IM_CORE_WARN("Tried to load ktx file with supercompression, which is currently not supported.");
		return false;
	}

	/**/

	ctx_.levels.resize(getLevelCount(ctx_.header));

	const streamsize levels_size = level_size * getLevelCount(ctx_.header);
	ctx_.source->get(header_size, levels_size, ctx_.levels.data(), bytes);

	if (bytes < levels_size) {
		return false;
	}

	/**/

	return true;
}

bool external::ktx::readData(cref<InternalContext> ctx_, level_type level_, std::span<_::byte> dst_) {

	if (level_ >= getLevelCount(ctx_.header)) {
		IM_CORE_WARN("Tried to load data from ktx file while mip level index is out of bound.");
		return false;
	}

	const auto& level = ctx_.levels[level_];
	if (level.byteSize == 0 || level.ucByteSize == 0) {
		return false;
	}

	if (*reinterpret_cast<ptr<const SuperCompressionScheme>>(&ctx_.header.ss) != SuperCompressionScheme::eNone) {
		IM_CORE_WARN("Tried to load data from ktx with supercompression, which is currently not supported.");
		return false;
	}

	/**/

	if (level.ucByteSize > dst_.size()) {
		IM_CORE_ERROR("Failed to load data from ktx, cause destination buffer is too small.");
		return false;
	}

	if (level.ucByteSize != level.byteSize) {
		IM_CORE_ERROR(
			"Failed to load data from ktx, cause compressed and uncompressed size are different while no super compression was provided."
		);
		return false;
	}

	/**/

	streamsize bytes = -1;
	streamoff offset = level.byteOff;

	while (
		(offset - level.byteOff) < level.byteSize &&
		ctx_.source->get(offset, level.byteSize - (offset - level.byteOff), dst_.data(), bytes)
	) {
		offset += bytes;
		bytes = -1;
	}

	/**/

	if (offset != level.byteOff + level.byteSize) {
		IM_CORE_ERROR("Failed to load data from ktx completely.");
		return false;
	}

	return true;
}

#pragma endregion

/**/

#pragma region Helper Functions

s32 calcMipDiff(cref<math::uivec3> left_, cref<math::uivec3> right_) {

	s32 mipDiff = 0L;
	math::uivec3 mipTestExtent = right_;

	if (left_.x >= mipTestExtent.x || left_.y >= mipTestExtent.y) {
		while (left_.x > mipTestExtent.x || left_.y > mipTestExtent.y) {
			mipTestExtent.x >>= 1uL;
			mipTestExtent.y >>= 1uL;
			++mipDiff;
		}

	} else {
		while (left_.x < mipTestExtent.x || left_.y < mipTestExtent.y) {
			mipTestExtent.x <<= 1uL;
			mipTestExtent.y <<= 1uL;
			--mipDiff;
		}
	}

	return mipDiff;
}

#pragma endregion
