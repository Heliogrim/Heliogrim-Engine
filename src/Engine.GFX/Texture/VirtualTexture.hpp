#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "Texture.hpp"
#include "__fwd.hpp"
#include "../Device/Device.hpp"
#include "../Memory/VirtualMemory.hpp"

namespace ember::engine::gfx::loader {
    class TextureLoader;
}

namespace ember::engine::gfx {

    class VirtualTexture final {
    public:
        friend class ::ember::engine::gfx::loader::TextureLoader;

    public:
        using this_type = VirtualTexture;

    public:
        VirtualTexture(
            mref<uptr<VirtualMemory>> memory_,
            u32 layers_,
            math::uivec3 extent_,
            TextureFormat format_,
            math::uivec2 mipLevels_,
            TextureType type_,
            /**/
            vk::Image vkImage_
        );

        VirtualTexture(
            mref<uptr<VirtualMemory>> memory_,
            u32 layers_,
            math::uivec3 extent_,
            TextureFormat format_,
            math::uivec2 mipLevels_,
            TextureType type_,
            /**/
            vk::Image vkImage_,
            /**/
            math::uivec3 granularity_,
            u32 mipTailFirstLod_,
            u32 mipTailSize_,
            u32 mipTailOffset_,
            u32 mipTailStride_
        );

        VirtualTexture(cref<this_type>) = delete;

        VirtualTexture(mref<this_type>) noexcept = delete;

        ~VirtualTexture();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        void tidy();

    private:
        uptr<VirtualMemory> _memory;

    private:
        Vector<ptr<VirtualTexturePage>> _pages;
        Vector<ptr<VirtualTexturePage>> _opaquePages;

    private:
        non_owning_rptr<VirtualTexturePage> makePage(
            u32 layer_,
            u32 mipLevel_,
            math::uivec3 tileOffset_,
            math::uivec3 tileExtent_
        );

        non_owning_rptr<VirtualTexturePage> makeOpaquePage(u32 layer_);

        void assureTiledPages(
            u32 layer_,
            math::uivec2 mipLevels_,
            math::uivec3 offset_,
            math::uivec3 extent_
        );

        void selectPages(
            math::uivec2 layers_,
            math::uivec2 mipLevels_,
            math::uivec3 offset_,
            math::uivec3 extent_,
            _Inout_ ref<Vector<non_owning_rptr<VirtualTexturePage>>> pages_
        );

    public:
        /**
         * Create an sub-resource view of this virtual texture
         *
         * @author Julius
         * @date 22.06.2022
         *
         * @returns A unique pointer to the created view.
         */
        [[nodiscard]] uptr<VirtualTextureView> makeView(
            u32 layer_,
            math::uivec2 mipLevels_
        );

        /**
         * Create an sub-resource view of this virtual texture
         *
         * @details This method is explicitly for usage with non-layered 2d textures.
         *  It will create a view for a spatial sub-resource of the original texture.
         *
         * @author Julius
         * @date 22.06.2022
         *
         * @returns A unique pointer to the created view.
         */
        [[nodiscard]] uptr<VirtualTextureView> makeSpatialView(
            math::uivec2 offset_,
            math::uivec2 extent_,
            math::uivec2 mipLevels_
        );

    private:
        u32 _layers;

    public:
        [[nodiscard]] u32 layers() const noexcept;

    private:
        math::uivec3 _extent;

    public:
        [[nodiscard]] math::uivec3::value_type width() const noexcept;

        [[nodiscard]] math::uivec3::value_type height() const noexcept;

        [[nodiscard]] math::uivec3::value_type depth() const noexcept;

    private:
        TextureFormat _format;

    public:
        [[nodiscard]] TextureFormat format() const noexcept;

    private:
        math::uivec2 _mipLevels;

    public:
        [[nodiscard]] math::uivec2::value_type minMipLevel() const noexcept;

        [[nodiscard]] math::uivec2::value_type mipLevels() const noexcept;

        [[nodiscard]] math::uivec2::value_type maxMipLevel() const noexcept;

    private:
        TextureType _type;

    public:
        [[nodiscard]] TextureType type() const noexcept;

    private:
        math::uivec3 _granularity;
        u32 _mipTailFirstLod;
        u32 _mipTailSize;
        u32 _mipTailOffset;
        u32 _mipTailStride;

    private:
        /**
         * Vulkan Virtual Texture
         */
        vk::Image _vkImage;

    public:
        [[nodiscard]] cref<vk::Image> vkImage() const noexcept;

    public:
        vk::ImageView _vkImageView;

    private:
        CompactSet<ptr<VirtualTexturePage>> _changedPages;
        CompactSet<ptr<VirtualTexturePage>> _changedOpaquePages;

    public:
        bool load(non_owning_rptr<VirtualTexturePage> page_);

        bool unload(non_owning_rptr<VirtualTexturePage> page_);

    private:
        Vector<vk::SparseImageMemoryBind> _bindings;
        Vector<vk::SparseMemoryBind> _opaqueBindings;
        vk::SparseImageMemoryBindInfo _bindData;
        vk::SparseImageOpaqueMemoryBindInfo _opaqueBindData;

    public:
        void unsafe_scan_changes();

        void updateBindingData();

    public:
        [[deprecated]] void enqueueBindingSync(const ptr<CommandQueue> queue_);
    };
}
