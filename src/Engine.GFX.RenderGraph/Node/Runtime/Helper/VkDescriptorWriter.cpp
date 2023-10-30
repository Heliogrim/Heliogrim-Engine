#include "VkDescriptorWriter.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Buffer/VirtualBuffer.hpp>
#include <Engine.GFX/Buffer/VirtualBufferView.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX/Texture/VirtualTexture.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

VkDescriptorWriter::VkDescriptorWriter(cref<vk::DescriptorSet> vkSet_) :
    _vkSet(vkSet_),
    _descriptorBufferInfos(),
    _descriptorImageInfos(),
    _writes() {}

void VkDescriptorWriter::update(cref<vk::Device> device_) const {
    device_.updateDescriptorSets(
        static_cast<u32>(_writes.size()),
        _writes.data(),
        0,
        nullptr
    );
}

void VkDescriptorWriter::storeUniform(index_type idx_, cref<Buffer> buffer_) {

    auto info = make_uptr<vk::DescriptorBufferInfo>(
        buffer_.buffer,
        0uLL,
        buffer_.size
    );

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eUniformBuffer,
        nullptr,
        info.get(),
        nullptr

    );
    _descriptorBufferInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::storeUniform(index_type idx_, cref<VirtualBuffer> buffer_) {

    auto info = make_uptr<vk::DescriptorBufferInfo>(
        buffer_.vkBuffer(),
        0uLL,
        buffer_.size()
    );

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eUniformBuffer,
        nullptr,
        info.get(),
        nullptr

    );
    _descriptorBufferInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::storeUniform(index_type idx_, cref<VirtualBufferView> view_) {

    auto info = make_uptr<vk::DescriptorBufferInfo>(
        view_.owner()->vkBuffer(),
        view_.offset(),
        view_.size()
    );

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eStorageBuffer,
        nullptr,
        info.get(),
        nullptr

    );
    _descriptorBufferInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::storeStorage(index_type idx_, cref<Buffer> buffer_) {

    auto info = make_uptr<vk::DescriptorBufferInfo>(
        buffer_.buffer,
        0uLL,
        buffer_.size
    );

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eStorageBuffer,
        nullptr,
        info.get(),
        nullptr

    );
    _descriptorBufferInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::storeStorage(index_type idx_, cref<VirtualBuffer> buffer_) {

    auto info = make_uptr<vk::DescriptorBufferInfo>(
        buffer_.vkBuffer(),
        0uLL,
        buffer_.size()
    );

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eStorageBuffer,
        nullptr,
        info.get(),
        nullptr

    );
    _descriptorBufferInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::storeStorage(index_type idx_, cref<VirtualBufferView> view_) {

    auto info = make_uptr<vk::DescriptorBufferInfo>(
        view_.owner()->vkBuffer(),
        view_.offset(),
        view_.size()
    );

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eUniformBuffer,
        nullptr,
        info.get(),
        nullptr

    );
    _descriptorBufferInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::store(index_type idx_, cref<Texture> texture_, cref<vk::Sampler> sampler_) {
    storeAs(idx_, texture_, sampler_, texture_.buffer()._vkLayout);
}

void VkDescriptorWriter::storeAs(
    index_type idx_,
    cref<Texture> texture_,
    cref<vk::Sampler> sampler_,
    cref<vk::ImageLayout> layout_
) {

    auto info = make_uptr<vk::DescriptorImageInfo>(sampler_, texture_.vkView(), layout_);

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eCombinedImageSampler,
        info.get(),
        nullptr,
        nullptr
    );
    _descriptorImageInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::store(index_type idx_, const ptr<const VirtualTexture> texture_, cref<vk::Sampler> sampler_) {
    storeAs(idx_, texture_, sampler_, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void VkDescriptorWriter::storeAs(
    index_type idx_,
    const ptr<const VirtualTexture> texture_,
    cref<vk::Sampler> sampler_,
    cref<vk::ImageLayout> layout_
) {

    auto info = make_uptr<vk::DescriptorImageInfo>(sampler_, texture_->_vkImageView, layout_);

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eCombinedImageSampler,
        info.get(),
        nullptr,
        nullptr
    );
    _descriptorImageInfos.push_back(_STD move(info));
}

void VkDescriptorWriter::store(
    index_type idx_,
    const ptr<const VirtualTextureView> texture_,
    cref<vk::Sampler> sampler_
) {
    auto info = make_uptr<vk::DescriptorImageInfo>(
        sampler_,
        reinterpret_cast<VkImageView>(texture_->vkImageView()),
        vk::ImageLayout::eShaderReadOnlyOptimal
    );

    _writes.emplace_back(
        _vkSet,
        idx_,
        0,
        1,
        vk::DescriptorType::eCombinedImageSampler,
        info.get(),
        nullptr,
        nullptr
    );
    _descriptorImageInfos.push_back(_STD move(info));
}
