#pragma once

#include "SceneNodeHead.hpp"
#include "SceneNode.hpp"
#include "../Storage/EmberSceneNodeStorage.hpp"

namespace ember::engine::scene {
    class SceneNodeFactory {

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 16.08.2021
         */
        SceneNodeFactory() = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  storage_ The storage.
         */
        SceneNodeFactory(sptr<EmberSceneNodeStorage> storage_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 16.08.2021
         */
        ~SceneNodeFactory() noexcept = default;

    private:
        /**
         * The node's storage
         */
        sptr<EmberSceneNodeStorage> _storage;

    public:
        /**
         * Gets the storage
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A sptr&lt;EmberSceneNodeStorage&gt;
         */
        [[nodiscard]] sptr<EmberSceneNodeStorage> storage() const noexcept;

    public:
        using factory_assemble_result = struct {
            SceneNodeHead head;
            ptr<SceneNode> body;
        };

    public:
        [[nodiscard]] factory_assemble_result assembleRoot() const;

        [[nodiscard]] factory_assemble_result assembleShadow() const;

        [[nodiscard]] factory_assemble_result assembleLoosy() const;

        [[nodiscard]] factory_assemble_result assembleSpartial() const;

        [[nodiscard]] factory_assemble_result assembleNatural() const;

    private:
        // Temporary
        inline static _STD atomic_uint32_t _nodeIdGen { _STD atomic_uint32_t(1) };
    };
}
