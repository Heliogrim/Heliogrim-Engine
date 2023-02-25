#pragma once

namespace ember::engine::scene {
    class SceneNodeHead;

    template <class ElementType_>
    class SceneNode;

    template <class PayloadType_>
    class ShadowSceneNode;

    template <class PayloadType_>
    class LoosySceneNode;

    template <class PayloadType_>
    class SpartialSceneNode;

    template <class PayloadType_>
    class NaturalSceneNode;

    template <class NodeStorageType_, class ElementStorageType_, class Traits_>
    class SceneNodeFactory;
}
