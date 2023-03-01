#pragma once

#include <Engine.ACS/ComponentTypeId.hpp>
#include <Engine.Common/Collection/BytellHashMap.hpp>

/**
 * Forward Declaration
 */
namespace hg {
    class SceneComponent;
}

namespace hg::engine::scene {
    class SceneNodeModel;
}

namespace hg::engine::scene {
    /**
     * TODO: Check whether we want a dynamic lookup container to enable more than fixed use modules
     */
    struct NodeTypeRegistryValue {
        // Equivalent to the constructor signature of SceneNodeModel
        using model_type = _STD function<ptr<SceneNodeModel>(ptr<SceneComponent>)>;

        model_type gfx;
        model_type pfx;
        model_type sfx;
    };

    class NodeTypeRegistry {
    public:
        NodeTypeRegistry() noexcept = default;

        ~NodeTypeRegistry() noexcept = default;

    private:
        static NodeTypeRegistryValue _empty;
        ska::bytell_hash_map<component_type_id, NodeTypeRegistryValue> _mapped;

        void sanitize(cref<component_type_id> typeId_);

    public:
        [[nodiscard]] cref<NodeTypeRegistryValue> get(cref<component_type_id> typeId_) noexcept;

        void setGfxModel(cref<component_type_id> typeId_, cref<NodeTypeRegistryValue::model_type> callback_);

        void setGfxModel(cref<component_type_id> typeId_, mref<NodeTypeRegistryValue::model_type> callback_);

        void setPfxModel(cref<component_type_id> typeId_, cref<NodeTypeRegistryValue::model_type> callback_);

        void setPfxModel(cref<component_type_id> typeId_, mref<NodeTypeRegistryValue::model_type> callback_);

        void setSfxModel(cref<component_type_id> typeId_, cref<NodeTypeRegistryValue::model_type> callback_);

        void setSfxModel(cref<component_type_id> typeId_, mref<NodeTypeRegistryValue::model_type> callback_);
    };
}
