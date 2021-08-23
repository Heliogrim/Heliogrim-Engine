#pragma once
#include <Engine.Common/Wrapper.hpp>
#include "SceneProxy.hpp"

namespace ember::engine::proxy {
    class SceneProxiedElementRef {
    private:
        /**
         * Scene Proxy
         */

    public:
        /**
         * Get a pointer to the SceneProxy
         *
         * @author Julius
         * @date 07.01.2021
         *
         * @returns A ptr&lt;SceneProxy&gt;
         */
        [[nodiscard]] ptr<SceneProxy> proxy() const noexcept;

    private:
        /**
         * Scene Element
         */

    public:
        /**
         * Get a pointer to the SceneElement
         *
         * @author Julius
         * @date 07.01.2021
         *
         * @returns A ptr&lt;SceneElement&gt;
         */
        [[nodiscard]] ptr<SceneElement> element() const noexcept;
    };
}
