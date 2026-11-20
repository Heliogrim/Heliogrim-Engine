#include "Device.hpp"

#include <cctype>
#include <Engine.Logging/Logger.hpp>

#include "../vkinc.hpp"

#ifdef _PROFILING
#endif

#include <Engine.Common/__macro.hpp>

#include "../Command/CommandQueue.hpp"
#include "../Library/Types.hpp"
#include "../Memory/VkAllocator.hpp"
#include "../Surface/Surface.hpp"

import Engine.Common;

using namespace hg::engine::graphics;
using namespace hg;

#ifdef _DEBUG
static std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MULTIVIEW_EXTENSION_NAME,
	VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME
};
#else
static std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MULTIVIEW_EXTENSION_NAME,
	VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME
};
#endif

static std::vector<const char*> validationLayers = {
	#ifdef _DEBUG
	"VK_LAYER_LUNARG_standard_validation",
	"VK_LAYER_KHRONOS_validation"
	#endif
	#ifdef ENABLE_RENDERDOC
	,
	"VK_LAYER_RENDERDOC_Capture"
	#endif
};

Device::Device(cref<Application> application_, const ptr<Surface> surface_) noexcept :
	_alloc(nullptr),
	_computeQueue(nullptr),
	_graphicsQueue(nullptr),
	_transferQueue(nullptr),
	_application(std::addressof(application_)),
	_surface(surface_),
	_device(nullptr),
	_physicalDevice(nullptr) {}

Device::~Device() = default;

static bool supportsSurface(cref<vk::PhysicalDevice> device_, const non_owning_rptr<const Surface> surface_) {

	if (not surface_) {
		return false;
	}

	vk::SurfaceCapabilitiesKHR caps = device_.getSurfaceCapabilitiesKHR(*surface_);
	const auto formats = device_.getSurfaceFormatsKHR(*surface_);
	const auto modes = device_.getSurfacePresentModesKHR(*surface_);

	return not formats.empty() && not modes.empty();
}

static bool supportsExtensions(cref<vk::PhysicalDevice> device_) {

	const auto properties { device_.enumerateDeviceExtensionProperties(nullptr) };
	Set<string> requiredExtensions { deviceExtensions.begin(), deviceExtensions.end() };

	for (const auto& extension : properties) {
		requiredExtensions.erase(extension.extensionName.operator std::string());
	}

	return requiredExtensions.empty();
}

static bool supportsHeadless(ref<const vk::PhysicalDevice> device_) {

	if (not supportsExtensions(device_)) {
		return false;
	}

	// Vulkan12Features may only be chained on 1.2 devices
	if (device_.getProperties().apiVersion < VK_API_VERSION_1_2) {
		return false;
	}

	const auto chain {
		device_.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceMultiviewFeatures>()
	};
	const auto& v12 { chain.get<vk::PhysicalDeviceVulkan12Features>() };
	const auto& multiview { chain.get<vk::PhysicalDeviceMultiviewFeatures>() };
	if (not v12.shaderInt8 || not v12.separateDepthStencilLayouts || not multiview.multiview) {
		return false;
	}

	// The headless targets are float render targets read back by transfer
	const vk::FormatFeatureFlags target { vk::FormatFeatureFlagBits::eColorAttachment | vk::FormatFeatureFlagBits::eTransferSrc };
	const auto tiling { device_.getFormatProperties(vk::Format::eR32G32B32A32Sfloat).optimalTilingFeatures };
	return (tiling & target) == target;
}

static string lowered(const std::string_view value_) {
	string result { value_ };
	std::ranges::transform(result, result.begin(), [](const unsigned char char_) { return static_cast<char>(std::tolower(char_)); });
	return result;
}

static vk::PhysicalDevice selectHeadless(ref<const std::vector<vk::PhysicalDevice>> candidates_) {

	const auto* const wanted { std::getenv("HELIOGRIM_TEST_GPU") };
	const auto filter { wanted != nullptr ? lowered(wanted) : string {} };

	vk::PhysicalDevice picked {};
	for (const auto& entry : candidates_) {

		if (not supportsHeadless(entry)) {
			continue;
		}

		const auto properties { entry.getProperties() };
		if (wanted != nullptr && not lowered(properties.deviceName.data()).contains(filter)) {
			continue;
		}

		if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
			return entry;
		}

		if (not picked) {
			picked = entry;
		}
	}

	if (not picked) {
		throw std::runtime_error("no GPU with these capabilities found");
	}

	return picked;
}

void Device::setup() {

	SCOPED_STOPWATCH

	const auto checkSurfaceSupport { _surface != nullptr };

	const std::vector<vk::PhysicalDevice> physicalCandidates = static_cast<vk::Instance>(*_application).enumeratePhysicalDevices();
	if (_application->headless()) {
		_physicalDevice = selectHeadless(physicalCandidates);

	} else {
		for (const auto& entry : physicalCandidates) {

			const bool extension { supportsExtensions(entry) };
			if (not extension) {
				continue;
			}

			const bool surface { supportsSurface(entry, _surface) };
			if (checkSurfaceSupport && not surface) {
				continue;
			}

			IM_DEBUG_LOG("Found vk::PhysicsDevice matching the requirements");
			_physicalDevice = entry;
			break;
		}
	}

	constexpr float defaultQueuePriority = 0.0F;
	std::vector<vk::DeviceQueueCreateInfo> dqcis {};

	auto requestedQueueType = vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eGraphics;

	types::QueueFamilyIndices qFamIndices {};
	if (requestedQueueType & vk::QueueFlagBits::eGraphics) {
		qFamIndices.graphicsFamily = getQueueFamilyIdx(vk::QueueFlagBits::eGraphics);

		vk::DeviceQueueCreateInfo dqCi = vk::DeviceQueueCreateInfo();
		dqCi.queueFamilyIndex = qFamIndices.graphicsFamily;
		dqCi.queueCount = 1;
		dqCi.pQueuePriorities = &defaultQueuePriority;

		dqcis.push_back(dqCi);
	}

	if (requestedQueueType & vk::QueueFlagBits::eCompute) {
		qFamIndices.computeFamily = getQueueFamilyIdx(vk::QueueFlagBits::eCompute);

		if (qFamIndices.computeFamily != qFamIndices.graphicsFamily) {
			vk::DeviceQueueCreateInfo dqCi = vk::DeviceQueueCreateInfo();
			dqCi.queueFamilyIndex = qFamIndices.computeFamily;
			dqCi.queueCount = 1;
			dqCi.pQueuePriorities = &defaultQueuePriority;

			dqcis.push_back(dqCi);
		}
	} else {
		qFamIndices.computeFamily = qFamIndices.graphicsFamily;
	}

	if (requestedQueueType & vk::QueueFlagBits::eTransfer) {
		qFamIndices.transferFamily = getQueueFamilyIdx(vk::QueueFlagBits::eTransfer);

		if (qFamIndices.transferFamily != qFamIndices.graphicsFamily && qFamIndices.transferFamily != qFamIndices.
			computeFamily) {
			vk::DeviceQueueCreateInfo dqCi = vk::DeviceQueueCreateInfo();
			dqCi.queueFamilyIndex = qFamIndices.transferFamily;
			dqCi.queueCount = 1;
			dqCi.pQueuePriorities = &defaultQueuePriority;

			dqcis.push_back(dqCi);
		}
	} else {
		qFamIndices.transferFamily = qFamIndices.graphicsFamily;
	}

	assert(qFamIndices.isComplete());

	const vk::PhysicalDevice physicalDevice { _physicalDevice };
	const auto features = physicalDevice.getFeatures();

	vk::DeviceCreateInfo dCi = vk::DeviceCreateInfo();
	dCi.queueCreateInfoCount = static_cast<uint32_t>(dqcis.size());
	dCi.pQueueCreateInfos = dqcis.data();
	dCi.pEnabledFeatures = &features;

	#ifdef _DEBUG
	//deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	#endif

	if (!deviceExtensions.empty()) {
		dCi.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		dCi.ppEnabledExtensionNames = deviceExtensions.data();
	}

	if (!validationLayers.empty()) {
		dCi.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		dCi.ppEnabledLayerNames = validationLayers.data();
	}

	vk::PhysicalDeviceMultiviewFeatures pdmf {};
	pdmf.multiview = VK_TRUE;

	vk::PhysicalDeviceVulkan12Features pdv12f {};
	pdv12f.shaderInt8 = VK_TRUE;
	pdv12f.separateDepthStencilLayouts = VK_TRUE;

	pdv12f.pNext = &pdmf;
	dCi.pNext = &pdv12f;

	vk::Device device {};

	if (_application->headless()) {
		// Device layers are deprecated and the loader warns about them, which the validation capture counts as a failure.
		dCi.enabledLayerCount = 0uL;
		dCi.ppEnabledLayerNames = nullptr;

		const auto result { physicalDevice.createDevice(&dCi, nullptr, &device) };
		if (result != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create the headless vk::Device");
		}

	} else {
		#ifdef _DEBUG
		assert(physicalDevice.createDevice(&dCi, nullptr, &device) == vk::Result::eSuccess);

		if (checkSurfaceSupport) {
			assert(physicalDevice.getSurfaceSupportKHR(qFamIndices.graphicsFamily, *_surface));
		}
		IM_DEBUG_LOG("Created vk::Device successfully.");

		#else

		physicalDevice.createDevice(&dCi, nullptr, &device);

		if (checkSurfaceSupport) {
			physicalDevice.getSurfaceSupportKHR(qFamIndices.graphicsFamily, *_surface);
		}
		#endif
	}

	_device = device;

	// Headless setup throws instead of exiting, so a failure past this point must not leak the device
	struct Rollback {
		ptr<Device> self;
		bool armed;

		~Rollback() {
			if (!armed) {
				return;
			}

			// Families may share one queue; release each distinct queue once, then the allocator, then the device.
			const auto graphics = self->_graphicsQueue;
			const auto compute = self->_computeQueue != graphics ? self->_computeQueue : nullptr;
			const auto transfer = self->_transferQueue != graphics && self->_transferQueue != self->_computeQueue ?
				self->_transferQueue :
				nullptr;

			for (const auto queue : { transfer, compute, graphics }) {
				if (queue != nullptr) {
					queue->destroy();
					delete queue;
				}
			}

			self->_graphicsQueue = nullptr;
			self->_computeQueue = nullptr;
			self->_transferQueue = nullptr;
			self->_alloc.reset();

			vkDestroyDevice(self->_device, nullptr);
			self->_device = nullptr;
		}
	} rollback { this, _application->headless() };

	/**
	 * Setup Allocator
	 */
	const auto vulkanLayerAlloc { make_sptr<memory::VkAllocator>(shared_from_this()) };
	auto fwdVulkanLayerAlloc { std::static_pointer_cast<memory::Allocator, memory::VkAllocator>(vulkanLayerAlloc) };
	_alloc = make_uptr<memory::GlobalPooledAllocator>(std::move(fwdVulkanLayerAlloc));

	/**
	 * Setup Queues
	 */

	// Graphic Queue
	_graphicsQueue = new CommandQueue(shared_from_this(), 0, UINT32_T(qFamIndices.graphicsFamily));

	if (qFamIndices.graphicsFamily == qFamIndices.computeFamily)
		_computeQueue = _graphicsQueue;

	if (qFamIndices.graphicsFamily == qFamIndices.transferFamily)
		_transferQueue = _graphicsQueue;

	// Compute Queue
	if (!_computeQueue) {
		_computeQueue = new CommandQueue(shared_from_this(), 0, UINT32_T(qFamIndices.computeFamily));
	}

	if (qFamIndices.computeFamily == qFamIndices.transferFamily) {
		_transferQueue = _computeQueue;
	}

	// Transfer Queue
	if (!_transferQueue) {
		_transferQueue = new CommandQueue(shared_from_this(), 0, UINT32_T(qFamIndices.transferFamily));
	}

	// Validate Queues
	assert(_graphicsQueue);
	assert(_computeQueue);
	assert(_transferQueue);
	IM_DEBUG_LOG("Created vk::Queue's successfully.");

	/**
	 * Setup
	 */
	_graphicsQueue->setup();

	if (!_computeQueue->vkQueue()) {
		_computeQueue->setup();
	}

	if (!_transferQueue->vkQueue()) {
		_transferQueue->setup();
	}
	IM_DEBUG_LOG("Acquired vk::Queue's successfully.");

	rollback.armed = false;
}

void Device::destroy() {

	//
	if (_transferQueue != _graphicsQueue) {
		_transferQueue->destroy();
		delete _transferQueue;
	}

	if (_computeQueue != _graphicsQueue) {
		_computeQueue->destroy();
		delete _computeQueue;
	}

	_graphicsQueue->destroy();
	delete _graphicsQueue;

	_transferQueue = nullptr;
	_computeQueue = nullptr;
	_graphicsQueue = nullptr;

	//
	_alloc.reset();

	/**/

	vkDevice().destroy();
	_device = nullptr;
}

vk::Device Device::vkDevice() const noexcept {
	return _device;
}

vk::PhysicalDevice Device::vkPhysicalDevice() const noexcept {
	return _physicalDevice;
}

nmpt<memory::GlobalPooledAllocator> Device::allocator() const noexcept {
	return _alloc.get();
}

ptr<CommandQueue> Device::computeQueue() const noexcept {
	return _computeQueue;
}

ptr<CommandQueue> Device::graphicsQueue() const noexcept {
	return _graphicsQueue;
}

ptr<CommandQueue> Device::transferQueue() const noexcept {
	return _transferQueue;
}

uint32_t Device::getQueueFamilyIdx(const vk::QueueFlagBits& flag_) const {
	auto props = vkPhysicalDevice().getQueueFamilyProperties();

	// Dedicated Compute
	if (flag_ & vk::QueueFlags(vk::QueueFlagBits::eCompute)) {
		for (uint32_t i = 0; i < props.size(); i++) {
			if (props[i].queueFlags & flag_ && !(props[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
				return i;
			}
		}
	}

	// Dedicated Transfer
	if (flag_ & vk::QueueFlags(vk::QueueFlagBits::eTransfer)) {
		for (uint32_t i = 0; i < props.size(); i++) {
			if (props[i].queueFlags & flag_ && !(props[i].queueFlags & vk::QueueFlagBits::eGraphics) && !(
				props[i].queueFlags & vk::QueueFlagBits::eCompute)) {
				return i;
			}
		}
	}

	// Shared Transfer & Compute
	if (flag_ & vk::QueueFlagBits::eTransfer || flag_ & vk::QueueFlagBits::eCompute) {
		for (u32 i = 0; i < props.size(); ++i) {

			auto& entry = props[i];
			if (entry.queueFlags & flag_ && !(entry.queueFlags & vk::QueueFlagBits::eGraphics)) {
				return i;
			}
		}
	}

	// First Matching
	for (uint32_t i = 0; i < props.size(); i++) {
		if (props[i].queueCount > 0 && props[i].queueFlags & flag_) {
			return i;
		}
	}

	throw std::runtime_error("Could not find matching queue family index.");
}
