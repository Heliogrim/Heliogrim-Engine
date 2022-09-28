#include "Device.hpp"
#include <unordered_set>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "../Command/CommandQueue.hpp"
#include "../Library/Types.hpp"
#include "../Surface/Surface.hpp"
#include "../Memory/VkAllocator.hpp"

using namespace ember::engine::gfx;
using namespace ember;

static std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static std::vector<const char*> validationLayers = {
    #ifdef _DEBUG
    "VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_KHRONOS_validation"
    #endif
    #ifdef ENABLE_RENDERDOC
			,"VK_LAYER_RENDERDOC_Capture"
    #endif
};

Device::Device(Application::const_reference_type application_, ptr<Surface> surface_) :
    _alloc(nullptr),
    _computeQueue(nullptr),
    _graphicsQueue(nullptr),
    _transferQueue(nullptr),
    _application(application_),
    _surface(surface_),
    _device(nullptr),
    _physicalDevice(nullptr) {}

Device::~Device() = default;

void Device::setup() {

    SCOPED_STOPWATCH

    // Device
    std::vector<vk::PhysicalDevice, std::allocator<vk::PhysicalDevice>> physicalCandidates = _application->
        enumeratePhysicalDevices();
    //QueueFamilyIndices indices;

    for (const auto& entry : physicalCandidates) {
        //indices = QueueFamilyIndices();
        //indices.graphicsFamily = _device->getQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
        //indices.presentFamily = NULL;

        bool swapChainAdequate = false;

        auto extensions = entry.enumerateDeviceExtensionProperties(nullptr);
        _STD unordered_set<std::string> requiredExtension(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : extensions) {
            requiredExtension.erase(extension.extensionName);
        }

        const bool extSupport = requiredExtension.empty();

        if (extSupport) {
            vk::SurfaceCapabilitiesKHR caps = entry.getSurfaceCapabilitiesKHR(*_surface);
            auto formats = entry.getSurfaceFormatsKHR(*_surface);
            auto modes = entry.getSurfacePresentModesKHR(*_surface);

            swapChainAdequate = (!formats.empty() && !modes.empty());
        }

        if (extSupport && swapChainAdequate) {
            _physicalDevice = entry;
            DEBUG_SNMSG(true, "VK", "Found vk::PhysicDevice matching requirements.")
            break;
        }
    }

    const float defaultQueuePriority = 0.0F;
    std::vector<vk::DeviceQueueCreateInfo> dqcis = std::vector<vk::DeviceQueueCreateInfo>(0);

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

    const auto features = _physicalDevice.getFeatures();

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

    #ifdef _DEBUG
    assert(_physicalDevice.createDevice(&dCi, nullptr, &_device) == vk::Result::eSuccess);
    assert(_physicalDevice.getSurfaceSupportKHR(qFamIndices.graphicsFamily, *_surface));
    DEBUG_SNMSG(true, "VK", "Created vk::Device successfully.")

    #else
    _physicalDevice.createDevice(&dCi, nullptr, &_device);
    _physicalDevice.getSurfaceSupportKHR(qFamIndices.graphicsFamily, *_surface);
    #endif

    /**
     * Setup Allocator
     */
    const auto vulkanLayerAlloc { make_sptr<memory::VkAllocator>(shared_from_this()) };
    auto fwdVulkanLayerAlloc { _STD static_pointer_cast<memory::Allocator, memory::VkAllocator>(vulkanLayerAlloc) };
    _alloc = make_uptr<memory::GlobalPooledAllocator>(_STD move(fwdVulkanLayerAlloc));

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
    DEBUG_SNMSG(true, "VK", "Created vk::Queue's successfully.")

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
    DEBUG_SNMSG(true, "VK", "Acquired vk::Queue's successfully.")
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
}

vk::Device& Device::vkDevice() const {
    return _device;
}

vk::PhysicalDevice& Device::vkPhysicalDevice() const {
    return _physicalDevice;
}

const non_owning_rptr<memory::GlobalPooledAllocator> Device::allocator() const noexcept {
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

uint32_t Device::getQueueFamilyIdx(const vk::QueueFlagBits& flag_) {
    auto props = _physicalDevice.getQueueFamilyProperties();

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
