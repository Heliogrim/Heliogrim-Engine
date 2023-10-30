#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/vkinc.hpp>

namespace hg::engine::gfx {
    class Buffer;
    class VirtualBuffer;
    class VirtualBufferView;
    class Texture;
    class VirtualTexture;
    class VirtualTextureView;
}

namespace hg::engine::gfx::render::graph {
    class VkDescriptorWriter {
    public:
        using index_type = u32;

    public:
        VkDescriptorWriter(cref<vk::DescriptorSet> vkSet_);

    private:
        vk::DescriptorSet _vkSet;

        Vector<uptr<vk::DescriptorBufferInfo>> _descriptorBufferInfos;
        Vector<uptr<vk::DescriptorImageInfo>> _descriptorImageInfos;
        Vector<vk::WriteDescriptorSet> _writes;

    public:
        void update(cref<vk::Device> device_) const;

    public:
        void storeUniform(index_type idx_, cref<Buffer> buffer_);

        void storeUniform(index_type idx_, cref<VirtualBuffer> buffer_);

        void storeUniform(index_type idx_, cref<VirtualBufferView> view_);

    public:
        void storeStorage(index_type idx_, cref<Buffer> buffer_);

        void storeStorage(index_type idx_, cref<VirtualBuffer> buffer_);

        void storeStorage(index_type idx_, cref<VirtualBufferView> view_);

    public:
        void store(index_type idx_, cref<Texture> texture_, cref<vk::Sampler> sampler_);

        void storeAs(
            index_type idx_,
            cref<Texture> texture_,
            cref<vk::Sampler> sampler_,
            cref<vk::ImageLayout> layout_
        );

    public:
        void store(index_type idx_, const ptr<const VirtualTexture> texture_, cref<vk::Sampler> sampler_);

        void storeAs(
            index_type idx_,
            const ptr<const VirtualTexture> texture_,
            cref<vk::Sampler> sampler_,
            cref<vk::ImageLayout> layout_
        );

        void store(index_type idx_, const ptr<const VirtualTextureView> texture_, cref<vk::Sampler> sampler_);
    };
}
