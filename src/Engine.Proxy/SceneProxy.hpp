#pragma once

#include <concepts>

#include "ProxyCategory.hpp"

namespace ember::engine::proxy {

    template <typename SceneProxyType_>
    concept IsSceneProxy = requires {
        { SceneProxyType_::proxy_category } -> std::same_as<proxy_category_tag>;
    };

    class SceneProxy { };
}
