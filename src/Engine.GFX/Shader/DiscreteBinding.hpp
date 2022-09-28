#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "ShaderBinding.hpp"
#include "../Buffer/__fwd.hpp"
#include "../Texture/__fwd.hpp"
#include "../Texture/TextureSampler.hpp"
namespace ember::engine::gfx::shader {

    class DiscreteBinding {
    public:
        using id_type = u32;

        using value_type = DiscreteBinding;
        using reference_type = DiscreteBinding&;
        using const_reference_type = const DiscreteBinding&;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.11.2020
         */
        DiscreteBinding();

        /**
         * Constructor
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @param  super_ The binding layout provided by the shader.
         * @param  vkSet_ The vulkan api descriptor set.
         */
        DiscreteBinding(const ptr<const ShaderBinding> super_, cref<vk::DescriptorSet> vkSet_) noexcept;

        /**
         * Copy Constructor
         */
        DiscreteBinding(cref<DiscreteBinding>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 09.03.2022
         *
         * @param other_ The other discrete binding to move.
         */
        DiscreteBinding(mref<DiscreteBinding> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 12.11.2020
         */
        ~DiscreteBinding();

    private:
        ptr<const ShaderBinding> _super;

    public:
        [[nodiscard]] cref<ShaderBinding> super() const noexcept;

    private:
        /**
         * 
         */
        vk::DescriptorSet _vkSet;

    public:
        [[nodiscard]] vk::DescriptorSet vkSet() const noexcept;

    public:
        /**
         * Stores the given buffer
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @param  buffer_ The buffer to store.
         */
        void store(const ref<Buffer>& buffer_);

        void store(const ref<VirtualBuffer> buffer_);

        void store(const ref<VirtualBufferView> view_);

    private:
        /**
         * 
         */
        TextureSampler _sampler;

    public:
        /**
         * Sets a sampler
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @param  sampler_ The sampler.
         */
        void setSampler(const ref<TextureSampler> sampler_) noexcept;

        /**
         * Stores the given texture
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @param  texture_ The texture to store.
         */
        void store(const ref<Texture> texture_);

        /**
         * Stores the given texture with specified layout
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @param  texture_ The texture to store.
         * @param  layout_ The textures's layout to use.
         */
        void storeAs(cref<Texture> texture_, cref<vk::ImageLayout> layout_);

    public:
        void store(const ptr<const VirtualTexture> texture_);
    };
}
