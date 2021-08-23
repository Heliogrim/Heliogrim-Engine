#pragma once

#include "Binding.hpp"

namespace ember::engine::gfx::shader {

    struct BindingUpdate {
        Binding::id_type id;

        Buffer* pBuffer;
        Texture* pTexture;
    };

    class BindingGroup {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @param  interval_ The interval.
         * @param  vkSet_ Set the vk belongs to.
         * @param  vkSetLayout_ The vk set layout.
         */
        BindingGroup(const BindingUpdateInterval interval_, const vk::DescriptorSet vkSet_,
            const vk::DescriptorSetLayout vkSetLayout_) noexcept;

        /**
         * Stores the given updates
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  updates_ The updates to store.
         */
        void store(const vector<BindingUpdate>& updates_);

    private:
        /**
         * Update Interval
         */
        BindingUpdateInterval _interval;

    public:
        /**
         * Gets the interval for binding updates
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A BindingUpdateInterval.
         */
        [[nodiscard]] BindingUpdateInterval interval() const noexcept;

    private:
        /**
         * Bindings 
         */
        vector<Binding> _bindings;

    public:
        /**
         * Gets the bindings
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const vector<Binding>& bindings() const noexcept;

        /**
         * Adds binding_
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  binding_ The binding to add.
         */
        void add(const Binding& binding_);

        /**
         * Determine if 'id_' exists
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  id_ The identifier.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool exists(const Binding::id_type id_) const noexcept;

        /**
         * Gets by identifier
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  id_ The identifier.
         *
         * @returns The by identifier.
         */
        [[nodiscard]] Binding& getById(const Binding::id_type id_);

    private:
        /**
         * Descriptor Set
         */
        vk::DescriptorSet _vkSet;

    public:
        /**
         * Vk set
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A vk::DescriptorSet.
         */
        [[nodiscard]] vk::DescriptorSet vkSet() const noexcept;

    private:
        /**
         * Descriptor Set Layout
         */
        vk::DescriptorSetLayout _vkSetLayout;

    public:
        /**
         * Vk set layout
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A vk::DescriptorSetLayout.
         */
        [[nodiscard]] vk::DescriptorSetLayout vkSetLayout() const noexcept;
    };
}
