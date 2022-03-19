#include "Factory.hpp"

using namespace ember::engine::gfx::shader;
using namespace ember::engine::gfx;
using namespace ember;

/**
 * Translate BindingType to vk::DescriptorType
 */
vk::DescriptorType vkTranslate(const BindingType type_) {
    switch (type_) {
        case BindingType::eUniformBuffer: {
            return vk::DescriptorType::eUniformBuffer;
        }
        case BindingType::eStorageBuffer: {
            return vk::DescriptorType::eStorageBuffer;
        }
        case BindingType::eImageSampler: {
            return vk::DescriptorType::eCombinedImageSampler;
        }
        default: {
            throw _STD runtime_error("Can not translate undefined `BindingType` to `vk::DescriptorType`.");
        }
    }
}

vk::ShaderStageFlags vkTranslate(const ShaderType type_) {

    vk::ShaderStageFlags flags { 0x0 };
    const ShaderTypes types { type_ };

    if (types & ShaderType::eAll) {
        flags |= vk::ShaderStageFlagBits::eAll;
    }

    if (types & ShaderType::eVertex) {
        flags |= vk::ShaderStageFlagBits::eVertex;
    }

    if (types & ShaderType::eFragment) {
        flags |= vk::ShaderStageFlagBits::eFragment;
    }

    if (types & ShaderType::eCompute) {
        flags |= vk::ShaderStageFlagBits::eCompute;
    }

    if (types & ShaderType::eGeometry) {
        flags |= vk::ShaderStageFlagBits::eGeometry;
    }

    if (types & ShaderType::eTessellation) {
        flags |= (vk::ShaderStageFlagBits::eTessellationControl | vk::ShaderStageFlagBits::eTessellationEvaluation);
    }

    if (type_ == ShaderType::eNone) {
        throw _STD runtime_error("Can not translate undefined `ShaderType` to `vk::ShaderStageFlagBits`.");
    }

    return flags;
}

Factory::Factory(cref<sptr<Device>> device_) noexcept :
    _device(device_) {}

Factory::~Factory() noexcept = default;

FactoryBuildResult Factory::build(_STD initializer_list<Prototype> list_) const {
    /**
     * Create pool information
     */
    Vector<vk::DescriptorPoolSize> sizes {};

    /**
     * Intervals, Sets and Layouts will be aligned
     */
    Vector<BindingUpdateInterval> intervals {};

    /**
     * Loop over every prototype
     */
    for (const auto& prototype : list_) {

        /**
         * Loop over every binding
         */
        for (const auto& binding : prototype.bindings()) {

            /**
             * Transform RequestType to DescriptorSize
             */
            sizes.push_back({
                vkTranslate(binding.type()),
                1
            });

            /**
             * Unique push_back to track required amount of descriptor sets
             */
            if (_STD find(intervals.begin(), intervals.end(), binding.updateInterval()) == intervals.end()) {
                intervals.push_back(binding.updateInterval());
            }

        }
    }

    /**
     * Create Descriptor Sets
     */
    Vector<ShaderBindingGroup> groups {};

    Vector<PrototypeBinding> bindings {};
    for (const auto& sp : list_) {
        for (const auto& bp : sp.bindings()) {

            /**
             * Check whether exists and match
             */
            const auto entry = _STD find_if(bindings.begin(), bindings.end(), [&bp](const PrototypeBinding& entry_) {
                return entry_.id() == bp.id() || entry_.name() == bp.name();
            });

            /**
             * Concatenate matching shader bindings or push new ones
             */
            if (entry != bindings.end()) {

                auto& unwrapped = *entry;

                assert(unwrapped.id() == bp.id());
                assert(unwrapped.name() == bp.name());

                unwrapped.shaderType() = static_cast<ShaderType>(
                    static_cast<u8>(unwrapped.shaderType()) | static_cast<u8>(sp.type())
                );

            } else {

                auto v { bp };
                v.shaderType() = sp.type();
                bindings.emplace_back(v);
            }

        }
    }

    for (const auto& interval : intervals) {
        /**
         * Collect Layout Bindings
         */
        Vector<vk::DescriptorSetLayoutBinding> layoutBindings {};

        for (const auto& binding : bindings) {

            if (binding.updateInterval() != interval) {
                continue;
            }

            const vk::DescriptorSetLayoutBinding dslb {
                binding.id(),
                vkTranslate(binding.type()),
                1,
                vkTranslate(binding.shaderType())
            };

            layoutBindings.push_back(dslb);
        }

        /**
         * Create DescriptorLayout
         */
        const vk::DescriptorSetLayoutCreateInfo info {
            vk::DescriptorSetLayoutCreateFlags {},
            static_cast<u32>(layoutBindings.size()),
            layoutBindings.data()
        };

        auto layout = _device->vkDevice().createDescriptorSetLayout(info);

        /**
         * Store Binding Group
         */
        groups.push_back({
            interval,
            layout
        });
    }

    /**
     * Create Shader
     */
    Vector<ptr<Shader>> shaders {};

    for (const auto& prototype : list_) {

        /**
         * Create Bindings
         */
        Vector<ShaderBinding> shaderBindings {};

        for (const auto& prototypeBinding : prototype.bindings()) {

            /**
             * Find Binding Group
             */
            ShaderBindingGroup* group = nullptr;
            for (auto& entry : groups) {
                if (entry.interval() == prototypeBinding.updateInterval()) {
                    group = &entry;

                    break;
                }
            }

            assert(group != nullptr);

            /**
             * Check whether binding does already exists
             */
            if (group->exists(prototypeBinding.id())) {

                #if _DEBUG
                auto value = group->getById(prototypeBinding.id());

                assert(
                    value.type() == prototypeBinding.type() &&
                    value.interval() == prototypeBinding.updateInterval()
                );
                #endif

                shaderBindings.push_back(group->getById(prototypeBinding.id()));
                continue;
            }

            /**
             * Instantiate Binding
             */
            ShaderBinding binding {
                prototypeBinding.type(),
                prototypeBinding.id(),
                prototypeBinding.updateInterval(),
                _device,
                group->vkSetLayout()
            };

            group->add(binding);
            shaderBindings.push_back(binding);
        }

        /**
         * Create vkShaderModule
         */
        const vk::ShaderModuleCreateInfo shaderInfo {
            vk::ShaderModuleCreateFlags(),
            prototype.codeSize(),
            static_cast<const u32*>(prototype.code())
        };

        const auto vkShader = _device->vkDevice().createShaderModule(shaderInfo);

        /**
         * Instantiate Shader
         */
        const auto shader = new Shader {
            prototype.type(),
            prototype.name(),
            shaderBindings,
            vkShader
        };

        shaders.push_back(shader);
    }

    return {
        groups,
        shaders
    };
}

cref<sptr<Device>> Factory::device() const noexcept {
    return _device;
}
