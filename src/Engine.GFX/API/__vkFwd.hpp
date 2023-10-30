#pragma once

namespace hg::_ {
    typedef struct _VkImage* VkImage;
    typedef struct _VkImageView* VkImageView;

    typedef struct _VkSemaphore* VkSemaphore;

    /**/

    typedef struct _VkShaderModule* VkShaderModule;

    /**/

    typedef struct _VkDescriptorSetLayout* VkDescriptorSetLayout;

    /**/

    typedef struct _VkGraphicsPipelineLayout* VkGraphicsPipelineLayout;
    typedef struct _VkGraphicsPipeline* VkGraphicsPipeline;

    typedef struct _VkComputePipelineLayout* VkComputePipelineLayout;
    typedef struct _VkComputePipeline* VkComputePipeline;

    /**/

    typedef struct _VkGraphicsRenderPass* VkGraphicsRenderPass;
    typedef struct _VkMeshRenderPass* VkMeshRenderPass;
    typedef struct _VkRaytracingRenderPass* VkRaytracingRenderPass;
}

namespace vk {}
