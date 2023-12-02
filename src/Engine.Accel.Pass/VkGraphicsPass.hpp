#pragma once
#include "GraphicsPass.hpp"

#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.GFX/vkinc.hpp>

namespace hg::engine::accel {
    class VkGraphicsPass final :
        public InheritMeta<VkGraphicsPass, GraphicsPass> {
    public:
        using this_type = VkGraphicsPass;

    public:
        constexpr VkGraphicsPass() noexcept = default;

        ~VkGraphicsPass() noexcept override;

    private:
        void tidy();

    private:
    public:
        Vector<vk::AttachmentDescription> _attachments;
        Vector<vk::AttachmentReference> _references;
        Vector<vk::SubpassDependency> _dependencies;

        _::VkGraphicsRenderPass _vkGraphicsPass;

    private:
    public:
        Vector<u32> _viewMasks;
        Vector<u32> _correlationMasks;

    public:
        [[nodiscard]] bool compatible(cref<AccelerationPipeline> pipeline_) const noexcept override;
    };
}
