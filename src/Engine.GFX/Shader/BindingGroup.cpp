#include "BindingGroup.hpp"

using namespace ember::engine::gfx::shader;
using namespace ember::engine::gfx;
using namespace ember;

BindingGroup::BindingGroup(const BindingUpdateInterval interval_, const vk::DescriptorSet vkSet_,
    const vk::DescriptorSetLayout vkSetLayout_) noexcept :
    _interval(interval_),
    _vkSet(vkSet_),
    _vkSetLayout(vkSetLayout_) {}

void BindingGroup::store(const vector<BindingUpdate>& updates_) {

    /**
     * Loop over update
     */
    for (const auto& update : updates_) {
        /**
         * Find Binding
         */
        auto entry = _STD find_if(_bindings.begin(), _bindings.end(), [id = update.id](const Binding& entry_) {
            return entry_.id() == id;
        });

        assert(entry != _bindings.end());

        /**
         * Switch by Type
         */
        switch (entry->type()) {
                /**
                 * Buffer Storing
                 */
            case BindingType::eStorageBuffer:
            case BindingType::eUniformBuffer: {
                #if _DEBUG
                assert(update.pBuffer);
                #endif

                entry->store(*update.pBuffer);
                break;
            }

                /**
                 * Texture Storing
                 */
            case BindingType::eImageSampler: {
                #if _DEBUG
                assert(update.pTexture);
                #endif

                entry->store(*update.pTexture);
                break;
            }

            default: {
                throw _STD runtime_error("`BindingType` is not dispatchable to store buffer or texture.");
            }
        }
    }
}

BindingUpdateInterval BindingGroup::interval() const noexcept {
    return _interval;
}

const vector<Binding>& BindingGroup::bindings() const noexcept {
    return _bindings;
}

void BindingGroup::add(const Binding& binding_) {
    #if _DEBUG
    assert(
        !exists(binding_.id()) &&
        binding_.vkSet() == _vkSet &&
        binding_.vkSetLayout() == _vkSetLayout
    );
    #endif

    _bindings.push_back(binding_);
}

bool BindingGroup::exists(const Binding::id_type id_) const noexcept {
    return _STD find_if(_bindings.begin(), _bindings.end(), [&id_](const Binding& entry_) {
        return entry_.id() == id_;
    }) != _bindings.end();
}

Binding& BindingGroup::getById(const Binding::id_type id_) {
    return *(
        _STD find_if(_bindings.begin(), _bindings.end(), [&id_](const Binding& entry_) {
            return entry_.id() == id_;
        })
    );
}

vk::DescriptorSet BindingGroup::vkSet() const noexcept {
    return _vkSet;
}

vk::DescriptorSetLayout BindingGroup::vkSetLayout() const noexcept {
    return _vkSetLayout;
}
