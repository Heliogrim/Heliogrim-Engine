#include "TextureLoader.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>

#include "../API/VkTranslate.hpp"
#include "../Buffer/Buffer.hpp"
#include "../Command/CommandBuffer.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember::engine::gfx;
using namespace ember;

TextureLoader::this_type TextureLoader::_this = nullptr;

Texture load_impl(const Url& url_,
    cref<sptr<Device>> device_) {

    gli::texture glitex = gli::load(url_);

    const auto lvlZeroExt = glitex.extent(0);
    math::ivec3 extent = {
        lvlZeroExt.x,
        lvlZeroExt.y,
        lvlZeroExt.z
    };

    const auto lvlZeroForm = glitex.format();
    vk::Format format;
    vk::ImageAspectFlags aspect;
    vk::ImageCreateFlags create = vk::ImageCreateFlags();
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc |
        vk::ImageUsageFlagBits::eTransferDst |
        vk::ImageUsageFlagBits::eSampled;

    switch (lvlZeroForm) {
            /**
             * R Formats (2D Images)
             *
             * Used for alpha, roughness, displacement
             */
        case gli::FORMAT_R16_SFLOAT_PACK16: {
            format = vk::Format::eR16Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_R32_SFLOAT_PACK32: {
            format = vk::Format::eR32Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }

            /**
             * D Formats (2D Images)
             *
             * Used for depth images or possible for alpha blending
             */
        case gli::FORMAT_D16_UNORM_PACK16: {
            format = vk::Format::eD16Unorm;
            aspect = vk::ImageAspectFlagBits::eDepth;
            break;
        }
        case gli::FORMAT_D32_SFLOAT_PACK32: {
            format = vk::Format::eD32Sfloat;
            aspect = vk::ImageAspectFlagBits::eDepth;
            break;
        }

            /**
             * DS Formats (2D Images)
             *
             * Used for depth stencil images
             */
        case gli::FORMAT_D16_UNORM_S8_UINT_PACK32: {
            format = vk::Format::eD16UnormS8Uint;
            aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        }
        case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64: {
            format = vk::Format::eD32SfloatS8Uint;
            aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        }

            /**
             * RGB Formats (2D Images)
             *
             * Used for color textures like albedo, sample maps or general image
             */
        case gli::FORMAT_RGB8_UNORM_PACK8: {
            format = vk::Format::eR8G8B8Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB8_SNORM_PACK8: {
            format = vk::Format::eR8G8B8Snorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB16_UNORM_PACK16: {
            format = vk::Format::eR16G16B16A16Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB32_UINT_PACK32: {
            format = vk::Format::eR32G32B32Uint;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB32_SFLOAT_PACK32: {
            format = vk::Format::eR32G32B32Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }

            /**
             * RGBA Formats (2D Images)
             *
             * Used for color textures with alpha like albedo + blending or sample maps
             */
        case gli::FORMAT_RGBA8_UNORM_PACK8: {
            format = vk::Format::eR8G8B8A8Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA8_SNORM_PACK8: {
            format = vk::Format::eR8G8B8A8Snorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA16_UNORM_PACK16: {
            format = vk::Format::eR16G16B16A16Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA32_UINT_PACK32: {
            format = vk::Format::eR32G32B32A32Uint;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA32_SFLOAT_PACK32: {
            format = vk::Format::eR32G32B32A32Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }

            /**
             * RGB(A) Formats (Compressed Cube Images)
             */
        case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16: {
            format = vk::Format::eAstc8x8UnormBlock;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8: {
            format = vk::Format::eEtc2R8G8B8UnormBlock;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16: {
            // TODO: Temporary
            format = vk::Format::eBc2UnormBlock;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        default: {
            throw _STD exception("Unresolved texture format.");
        }
    }

    const auto faceCount = glitex.faces();
    if (faceCount == 6) {
        create = vk::ImageCreateFlagBits::eCubeCompatible;
    }

    /**
     * Create Texture
     */
    auto tf = TextureFactory::get();

    const TextureBuildPayload payload {
        math::uivec3 {
            static_cast<u32>(extent.x),
            static_cast<u32>(extent.y),
            static_cast<u32>(extent.z)
        },
        TextureFormat { api::vkTranslateFormat(format) },
        static_cast<u32>(glitex.levels()),
        create & vk::ImageCreateFlagBits::eCubeCompatible ?
            TextureType::eCube :
            TextureType::e2d,
        aspect,
        usage,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        vk::SharingMode::eExclusive
    };

    Texture result = tf->build(payload);

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

    const vk::MemoryRequirements mr = device_->vkDevice().getBufferMemoryRequirements(stage.buffer);

    const vk::MemoryAllocateInfo mai {
        mr.size,
        get_memory_type(device_->vkPhysicalDevice(), mr.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    stage.memory = device_->vkDevice().allocateMemory(mai);

    assert(stage.buffer);
    assert(stage.memory);

    stage.align = mr.alignment;
    stage.size = mr.size;
    stage.memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    stage.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;

    /**
     *
     */
    stage.bind();
    stage.map();
    assert(stage.mapped);

    /**
     * Copy Data
     */
    memcpy(stage.mapped, glitex.data(), glitex.size());

    /**
     *
     */
    stage.unmap();

    /**
     * Fetch Region per Layer
     */
    Vector<vk::BufferImageCopy> regions {};
    uint32_t offset = 0;

    if (create & vk::ImageCreateFlagBits::eCubeCompatible) {

        const gli::texture_cube ct { glitex };

        for (uint32_t face = 0; face < glitex.faces(); face++) {
            for (uint32_t level = 0; level < result.mipLevels(); ++level) {

                const auto se = ct[face][level].extent();

                vk::BufferImageCopy copy {
                    offset,
                    0,
                    0,
                    {
                        aspect,
                        level,
                        face,
                        result.depth()
                    },
                    vk::Offset3D(),
                    vk::Extent3D {
                        static_cast<uint32_t>(se.x),
                        static_cast<uint32_t>(se.y),
                        result.depth()
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
        for (uint32_t level = 0; level < result.mipLevels(); ++level) {
            vk::BufferImageCopy copy {
                offset,
                0,
                0,
                {
                    aspect,
                    level,
                    0,
                    result.type() == TextureType::e2dArray ? result.depth() : 1
                },
                vk::Offset3D(),
                vk::Extent3D {
                    result.width() / (0x1 << level),
                    result.height() / (0x1 << level),
                    result.type() == TextureType::e2dArray ? 1 : result.depth()
                }
            };

            regions.push_back(copy);
            offset += static_cast<uint32_t>(glitex.size(level));
        }
    }

    /**
     * Copy Data to Image
     */
    auto pool = device_->transferQueue()->pool();
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
        result.buffer().image(),
        {
            aspect,
            0,
            result.mipLevels(),
            0,
            result.layer()
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &simb
    );

    result.buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;

    for (auto& entry : regions) {
        cmd.copyBufferToImage(
            stage, result.buffer(), entry
        );
    }

    /**
     * Restore Layout
     */
    vk::ImageMemoryBarrier eimb {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eTransferSrcOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        result.buffer().image(),
        {
            aspect,
            0,
            result.mipLevels(),
            0,
            result.layer()
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &eimb
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();

    /**
     * Cleanup
     */
    stage.destroy();
    glitex.clear();

    result.buffer()._vkLayout = vk::ImageLayout::eTransferSrcOptimal;
    return result;
}

TextureLoader::const_reference_type TextureLoader::make(cref<sptr<Device>> device_) {
    if (!_this)
        _this = _STD move(
            _STD unique_ptr<TextureLoader>(new TextureLoader(device_))
        );
    return *_this;
}

TextureLoader::const_reference_type TextureLoader::get() {
    return *_this;
}

void TextureLoader::destroy() {
    TextureLoader::_this = nullptr;
}

TextureLoader::TextureLoader(cref<sptr<Device>> device_) :
    _device(device_) {}

ember::concurrent::promise<Texture> TextureLoader::load(
    const Url& url_) const {
    concurrent::promise<Texture> p {
        [_device = _device, url_ = url_]() {
            return load_impl(url_, _device);
        }
    };

    return p;
}

#if FALSE

__Texture2d TextureLoader::load2dToBuffer(const gli::texture& tex_, const vk::Format& format_, const __Queue& queue_,
	const vk::ImageUsageFlags& imageFlags_, const vk::ImageLayout& imageLayout_, const bool& staging_) const {
	gli::texture2d tex = gli::texture2d(tex_);

	__Texture2d texture = __internal::types::shared<Texture2d>(_device, tex[0].extent().x, tex[0].extent().y,
		tex.levels());
	texture->meta().usage(imageFlags_);
	texture->meta().format(format_);

	const vk::FormatProperties formatProps = _device->physDev.getFormatProperties(format_);

	// Use a separate command buffer for texture loading
	const vk::CommandBuffer copyCmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary,
		queue_->familyIndex(), true);
	copyCmd.begin(vk::CommandBufferBeginInfo());

	if (staging_) {
		// Comment: vk::MemoryPropertyFlagBits::eDeviceLocal will cause a unaccessible eHostVisible and eHostCoherent
// Comment: !!! Do not use DeviceLocal and HostVisible/Coherent at once
//texture->memoryFlags(texture->memoryFlags()/* | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent */);

		/**
		 *	Create new texture
		 */
		texture->init(*_initializer);
#ifdef _DEBUG
		DEBUG_NMSG("GUT", "Created target 2d texture while loading.")
#endif

		/**
		 *	Copy data to texture
		 */
		Buffer staging = _device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::
			MemoryPropertyFlagBits::eHostCoherent, tex.size());
		staging.bind();

#ifdef _DEBUG
		DEBUG_NMSG("GUT", "Created staging buffer for 2d texture loading.")
#endif

		staging.map();
		staging.copyTo(tex.data(), tex.size());
		staging.unmap();

		// Setup buffer copy regions for each mip level
		std::vector<vk::BufferImageCopy> copyRegions = std::vector<vk::BufferImageCopy>(0);
		uint32_t offset = 0;

		for (uint32_t i = 0; i < texture->meta().mipLevels(); i++) {
			vk::BufferImageCopy region = vk::BufferImageCopy();
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = i;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = static_cast<uint32_t>(tex[i].extent().x);
			region.imageExtent.height = static_cast<uint32_t>(tex[i].extent().y);
			region.imageExtent.depth = 1;
			region.bufferOffset = offset;

			copyRegions.push_back(region);

			offset += static_cast<uint32_t>(tex[i].size());
		}

		if (!(texture->meta().usage() & vk::ImageUsageFlagBits::eTransferDst))
			texture->meta().usage(texture->meta().usage() | vk::ImageUsageFlagBits::eTransferDst);

		texture->toImageLayout(vk::ImageLayout::eTransferDstOptimal, copyCmd);
		copyCmd.copyBufferToImage(staging.buffer, texture->image().image, vk::ImageLayout::eTransferDstOptimal,
			static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
		texture->toImageLayout(imageLayout_, copyCmd);

		_device->flushCmdBuffer(copyCmd, queue_, true);
		staging.destroy();
	} else {
		// Bug: Broke - Image Raw Memory is not the same as buffer raw memory and raw data of image in file
// TODO: lookup whether raw image memory is fillable by ktx file format or whether stage algorithm is required
		/**
		 *	Create new texture
		 */
		assert(formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);
		texture->image().memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::
			eHostCoherent;

		texture->init(*_initializer);
#ifdef _DEBUG
		DEBUG_NMSG("GUT", "Created visible target 2d texture while loading.")
#endif

		/**
		 *	Copy data to texture
		 */
		vk::ImageSubresource subRes = vk::ImageSubresource();
		subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
		subRes.mipLevel = 0;
		subRes.arrayLayer = 0;

		const vk::MemoryRequirements memReq = _device->logDev.getImageMemoryRequirements(texture->image().image);
		vk::SubresourceLayout subResLayout = _device->logDev.getImageSubresourceLayout(texture->image().image, subRes);
		void* data;

		vk::CommandBuffer cmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, queue_->familyIndex(),
			true);
		cmd.begin(vk::CommandBufferBeginInfo());
		texture->toImageLayout(vk::ImageLayout::eTransferDstOptimal, cmd);
		_device->flushCmdBuffer(cmd, _device->transferQueue, true);

		texture->image().map();
		texture->image().copyTo(tex[subRes.mipLevel].data(), tex[subRes.mipLevel].size());
		texture->image().unmap();

		texture->toImageLayout(imageLayout_, copyCmd);

		_device->flushCmdBuffer(copyCmd, queue_, true);
	}

	texture->update();
	return texture;
}

__TextureCubemap TextureLoader::loadCubeToBuffer(const gli::texture& tex_, const vk::Format& format_,
	const __Queue& queue_, const vk::ImageUsageFlags& imageFlags_, const vk::ImageLayout& imageLayout_,
	const bool& staging_) const {
	gli::texture_cube tex = gli::texture_cube(tex_);

	__TextureCubemap texture = __internal::types::shared<TextureCubemap>(_device, tex[0].extent().x, tex[0].extent().y,
		tex[0].levels());
	texture->meta().flags(vk::ImageCreateFlagBits::eCubeCompatible);
	texture->meta().format(format_);
	texture->meta().usage(imageFlags_);

	texture->samplerMeta().addressMode(
		vk::SamplerAddressMode::eClampToEdge,
		vk::SamplerAddressMode::eClampToEdge,
		vk::SamplerAddressMode::eClampToEdge
	);

	vk::CommandBuffer cmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, queue_->familyIndex(), true);
	cmd.begin(vk::CommandBufferBeginInfo());

	if (!(texture->meta().usage() & vk::ImageUsageFlagBits::eTransferDst))
		texture->meta().usage(texture->meta().usage() | vk::ImageUsageFlagBits::eTransferDst);

	/**
	 *	Create new texture
	 */
	texture->init(*_initializer);

	/**
	 *	Copy data to texture
	 */
	Buffer staging = _device->createBuffer(
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, tex.size(), tex.data());
	staging.bind();

	std::vector<vk::BufferImageCopy> regions;
	uint32_t offset = 0;

	for (int32_t face = 0; face < texture->meta().depth(); face++)
		for (uint32_t level = 0; level < texture->meta().mipLevels(); level++) {
			vk::BufferImageCopy region = vk::BufferImageCopy();
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = level;
			region.imageSubresource.baseArrayLayer = face;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = vk::Extent3D(
				UINT32_T(tex[face][level].extent().x), UINT32_T(tex[face][level].extent().y), 1);
			region.bufferOffset = offset;

			regions.push_back(region);
			offset += UINT32_T(tex[face][level].size());
		}

	vk::ImageSubresourceRange isr = vk::ImageSubresourceRange();
	isr.aspectMask = vk::ImageAspectFlagBits::eColor;
	isr.baseMipLevel = 0;
	isr.levelCount = texture->meta().mipLevels();
	isr.layerCount = texture->meta().depth();

	texture->toImageLayout(vk::ImageLayout::eTransferDstOptimal, cmd);
	cmd.copyBufferToImage(staging.buffer, texture->image().image, vk::ImageLayout::eTransferDstOptimal,
		UINT32_T(regions.size()), regions.data());
	texture->toImageLayout(imageLayout_, cmd);
	
	_device->flushCmdBuffer(cmd, queue_, true);

	staging.destroy();

	return texture;
}

TextureLoader::future_2d_type TextureLoader::load2dFromFile(const _STD string& file_, const vk::Format& format_, const
	__Queue& queue_, const vk::ImageUsageFlags& flags_, const vk::ImageLayout& layout_, const bool& staging_) const {
	gli::texture2d glit(gli::load(file_));
#ifdef _DEBUG
	DEBUG_NMSG("GUT", "Loading 2d texture from file ('" + file_ + "').");
#endif

	return tasking::async<__Texture2d>([=] {
		return this->load2dToBuffer(glit, format_, queue_, flags_, layout_, staging_);
	});
}

TextureLoader::future_cube_type TextureLoader::loadCubeFromFile(const _STD string& file_, const vk::Format& format_,
	const
	__Queue& queue_, const vk::ImageUsageFlags& flags_, const vk::ImageLayout& layout_, const bool& staging_) const {
	gli::texture_cube glit(gli::load(file_));
#ifdef _DEBUG
	DEBUG_NMSG("GUT", "Loading 2d texture from file ('" + file_ + "').");
#endif
	return tasking::async<__TextureCubemap>([=] {
		return this->loadCubeToBuffer(glit, format_, queue_, flags_, layout_, staging_);
	});
}

#endif
