#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.GFX/vkinc.hpp>

namespace hg::engine::gfx {
    class StorageBufferView;
    class UniformBufferView;
    class Buffer;
    class SparseBuffer;
    class SparseBufferView;
    class Texture;
    class TextureView;
    class SparseTexture;
    class SparseTextureView;
    class TextureLikeObject;
    class TextureSampler;
}

namespace hg::engine::render::graph {
    class VkDescriptorWriter {
    public:
        using index_type = u32;

    public:
        VkDescriptorWriter(cref<vk::DescriptorSet> vkSet_);

    private:
        vk::DescriptorSet _vkSet;

        bool _addressPatched;
        Vector<vk::DescriptorBufferInfo> _descriptorBufferInfos;
        Vector<vk::DescriptorImageInfo> _descriptorImageInfos;
        Vector<vk::DescriptorImageInfo> _descriptorSamplerInfos;
        Vector<vk::WriteDescriptorSet> _writes;

    private:
        ptrdiff_t emplaceBufferInfo(mref<vk::DescriptorBufferInfo> info_);

        ptrdiff_t emplaceImageInfo(mref<vk::DescriptorImageInfo> info_);

        ptrdiff_t emplaceSamplerInfo(mref<vk::DescriptorImageInfo> info_);

        void patchInfoAddress();

    public:
        void update(cref<vk::Device> device_);

        void reset(cref<vk::DescriptorSet> nextVkSet_);

    public:
        void storeUniform(index_type idx_, cref<gfx::Buffer> buffer_);

        void storeUniform(index_type idx_, cref<gfx::SparseBuffer> buffer_);

        void storeUniform(index_type idx_, cref<gfx::SparseBufferView> view_);

        void storeUniform(index_type idx_, nmpt<const gfx::UniformBufferView> view_);

    public:
        void storeStorage(index_type idx_, cref<gfx::Buffer> buffer_);

        void storeStorage(index_type idx_, cref<gfx::SparseBuffer> buffer_);

        void storeStorage(index_type idx_, cref<gfx::SparseBufferView> view_);

        void storeStorage(index_type idx_, nmpt<const gfx::StorageBufferView> view_);

    public:
        void storeTexture(index_type idx_, nmpt<const gfx::TextureLikeObject> texture_);

        void storeTextureSampler(index_type idx_, nmpt<const gfx::TextureSampler> sampler_);

    public:
        void store(index_type idx_, cref<gfx::Texture> texture_, cref<vk::Sampler> sampler_);

        void storeAs(
            index_type idx_,
            cref<gfx::Texture> texture_,
            cref<vk::Sampler> sampler_,
            cref<vk::ImageLayout> layout_
        );

    public:
        void store(index_type idx_, const ptr<const gfx::SparseTexture> texture_, cref<vk::Sampler> sampler_);

        void storeAs(
            index_type idx_,
            const ptr<const gfx::SparseTexture> texture_,
            cref<vk::Sampler> sampler_,
            cref<vk::ImageLayout> layout_
        );

        void store(index_type idx_, const ptr<const gfx::SparseTextureView> texture_, cref<vk::Sampler> sampler_);
    };
}
