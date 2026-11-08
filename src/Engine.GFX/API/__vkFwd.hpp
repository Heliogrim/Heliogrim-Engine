#pragma once

#include <concepts>

/**
 * Vulkan forward declarations, cheap to parse in headers.
 *
 * Each declaration repeats the one of vulkan_core.h / vulkan-hpp exactly; vkinc.hpp includes this file first,
 * so every TU parsing the real headers verifies them (a mismatching redeclaration is a hard error).
 *
 * Usable by value: C handles (`VkBuffer`), C flags (`VkBufferUsageFlags`) and `vk::` enums.
 * Declaration only: `vk::` handle classes, `vk::Flags<>` and `vk::` structs.
 */

static_assert(sizeof(void*) == 8, "C handle declarations assume VK_USE_64_BIT_PTR_DEFINES.");

/* C handles */

typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkCommandPool_T* VkCommandPool;

typedef struct VkBuffer_T* VkBuffer;
typedef struct VkBufferView_T* VkBufferView;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;
typedef struct VkSampler_T* VkSampler;
typedef struct VkDeviceMemory_T* VkDeviceMemory;

typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkFence_T* VkFence;

typedef struct VkShaderModule_T* VkShaderModule;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;

/* C flags */

typedef uint32_t VkFlags;
typedef VkFlags VkBufferUsageFlags;
typedef VkFlags VkCullModeFlags;
typedef VkFlags VkImageAspectFlags;
typedef VkFlags VkImageCreateFlags;
typedef VkFlags VkImageUsageFlags;
typedef VkFlags VkMemoryPropertyFlags;
typedef VkFlags VkPipelineStageFlags;
typedef VkFlags VkQueueFlags;
typedef VkFlags VkSampleCountFlags;

namespace vk {
	template <typename BitType>
	class Flags;

	/* Enums */

	enum class BorderColor;
	enum class CompareOp;
	enum class DescriptorType;
	enum class Filter;
	enum class Format;
	enum class FrontFace;
	enum class ImageLayout;
	enum class ImageTiling;
	enum class PolygonMode;
	enum class PrimitiveTopology;
	enum class SamplerAddressMode;
	enum class SamplerMipmapMode;
	enum class SharingMode;
	enum class StencilOp;
	enum class VertexInputRate;

	enum class BufferUsageFlagBits : VkBufferUsageFlags;
	enum class CullModeFlagBits : VkCullModeFlags;
	enum class ImageAspectFlagBits : VkImageAspectFlags;
	enum class ImageCreateFlagBits : VkImageCreateFlags;
	enum class ImageUsageFlagBits : VkImageUsageFlags;
	enum class MemoryPropertyFlagBits : VkMemoryPropertyFlags;
	enum class PipelineStageFlagBits : VkPipelineStageFlags;
	enum class QueueFlagBits : VkQueueFlags;
	enum class SampleCountFlagBits : VkSampleCountFlags;

	using BufferUsageFlags = Flags<BufferUsageFlagBits>;
	using ImageAspectFlags = Flags<ImageAspectFlagBits>;
	using ImageCreateFlags = Flags<ImageCreateFlagBits>;
	using ImageUsageFlags = Flags<ImageUsageFlagBits>;
	using MemoryPropertyFlags = Flags<MemoryPropertyFlagBits>;
	using PipelineStageFlags = Flags<PipelineStageFlagBits>;

	/* Handle classes */

	class Instance;
	class PhysicalDevice;
	class Device;
	class Queue;
	class Buffer;
	class Image;
	class ImageView;
	class Sampler;
	class SurfaceKHR;
	class DeviceMemory;
	class Semaphore;
	class Fence;

	/* Structs */

	struct SparseMemoryBind;
	struct SubpassDependency;
}

namespace hg {
	/**
	 * Holds a `vk::Flags<BitType>` by value where only these declarations are visible.
	 * Converts implicitly from `BitType` and to/from `vk::Flags<BitType>` where the full headers are visible.
	 */
	template <typename BitType>
	struct VkFlagStore {
		VkFlags mask = 0;

		constexpr VkFlagStore() noexcept = default;

		constexpr VkFlagStore(BitType bit_) noexcept :
			mask(static_cast<VkFlags>(bit_)) {}

		template <std::same_as<::vk::Flags<BitType>> Flags_>
		constexpr VkFlagStore(const Flags_& flags_) noexcept :
			mask(static_cast<VkFlags>(flags_)) {}

		template <std::same_as<::vk::Flags<BitType>> Flags_>
		constexpr operator Flags_() const noexcept {
			return Flags_ { mask };
		}
	};
}

namespace hg::_ {
	/* Legacy aliases; the role-specific names map onto the shared Vulkan handle. */

	using VkBuffer = ::VkBuffer;
	using VkBufferView = ::VkBufferView;

	using VkImage = ::VkImage;
	using VkImageView = ::VkImageView;

	using VkSemaphore = ::VkSemaphore;

	/**/

	using VkShaderModule = ::VkShaderModule;

	/**/

	using VkDescriptorSetLayout = ::VkDescriptorSetLayout;
	using VkDescriptorSet = ::VkDescriptorSet;

	/**/

	using VkGraphicsPipelineLayout = ::VkPipelineLayout;
	using VkGraphicsPipeline = ::VkPipeline;

	using VkComputePipelineLayout = ::VkPipelineLayout;
	using VkComputePipeline = ::VkPipeline;

	/**/

	using VkGraphicsRenderPass = ::VkRenderPass;
	using VkMeshRenderPass = ::VkRenderPass;
	using VkRaytracingRenderPass = ::VkRenderPass;
}
