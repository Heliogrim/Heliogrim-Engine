#include "MipMap.hpp"

#include "../Command/CommandBuffer.hpp"

using namespace hg::engine::gfx::generator;
using namespace hg::engine::gfx;
using namespace hg;

void MipMapGenerator::generate(Texture& texture_, cref<sptr<Device>> device_, CommandQueue& queue_) {
    /**
     * Get count of mip levels
     */
    const u32 extent = MAX(texture_.width(), texture_.height());
    texture_.mipLevels() = calcMipLevel(extent);

    // TODO:
    TextureBuffer dst {};

    /**
     * Build Command Buffer
     */
    CommandBuffer cmd = queue_.pool()->make();

    /**
     * Acquire Pool and Begin Recording
     */
    cmd.lck().acquire();
    cmd.begin();

    /**
     * Translate Image Memory Layout to Transfer Src and Transfer Dst
     */
    // TODO:

    /**
     * Copy Level 0
     */
    vk::ImageCopy region {};

    region.extent = vk::Extent3D(texture_.width(), texture_.height(), 1);
    region.srcOffset = vk::Offset3D();
    region.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.layerCount = 1;

    region.dstOffset = vk::Offset3D();
    region.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.layerCount = 1;

    cmd.copyImage(texture_.buffer(), dst, region);

    /**
     * Create Image Blit Src and Dst description
     */
    vk::ImageBlit blit {};

    blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    blit.srcSubresource.layerCount = 1uL;
    blit.srcSubresource.mipLevel = 0;
    blit.srcOffsets[0] = vk::Offset3D();
    blit.srcOffsets[1] = vk::Offset3D(texture_.width(), texture_.height(), 1uL);

    for (u32 level = 1; level < texture_.mipLevels(); ++level) {

        const u32 dstW = texture_.width() >> level;
        const u32 dstH = texture_.height() >> level;

        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.layerCount = 1uL;
        blit.dstSubresource.mipLevel = level;
        blit.dstOffsets[0] = vk::Offset3D();
        blit.dstOffsets[1] = vk::Offset3D(dstW, dstH, 1uL);

        cmd.blit(blit, texture_.buffer(), dst);
    }

    /**
     * Translate Image Memory Layout from Transfer Dst to Src Texture original layout
     */
    // TODO:

    /**
     * Release Pool and Execute Command Buffer
     */
    cmd.lck().release();
    // TODO: Replace with sync command, cause submit is async submitting of command batch with signals
    queue_.submitWait({ cmd });
    cmd.release();

    /**
     * Replace Texture internal buffer
     */
    texture_.buffer().destroy();
    texture_.buffer() = dst;
}

u32 MipMapGenerator::calcMipLevel(const u32 extent_) const noexcept {
    return UINT32_T(
        std::floor(
            std::log2(extent_)
        ) + 1
    );
}
