#pragma once
#include <Engine.Common/Flag.hpp>

namespace hg::engine::scene {
    /**
     * Values that represent scene node states
     *
     *  [Shadow] : Nullable State
     *
     *      -> has no transform itself
     *      -> has no bounding itself
     * 
     *  [Loosy] : Loosy Tree ( e.g. Loosy Octree )
     *
     *      -> has transform itself
     *      -> has bounding itself
     *      -> children are possibly double referenced
     *      -> children share some bounding
     * 
     *  [Spartial] : Spartial Tree ( e.g. Custom Octree )
     *
     *      -> has transform itself
     *      -> has bounding itself
     *      -> children are unique referenced
     *      -> children have distinct bounding
     * 
     *  [Natural] : Natural Tree ( e.g. Strict Octree with Center-Split )
     *
     *      -> has transform itself
     *      -> has bounding itself
     *      -> children are unique referenced
     *      -> children have distinct and even bounding
     */
    enum class SceneNodeState: hg::u8 {
        eShadow = 0x00,
        eLoosy = 0x01,
        eSpartial = 0x02,
        eNatural = 0x03
    };

    typedef hg::Flag<SceneNodeState> SceneNodeStates;

    struct scene_node_shared_traits {
        using container_size_type = u8;
        using container_diff_type = s8;

        static constexpr container_size_type max_nodes_per_layer = 8;
    };

    template <SceneNodeState State_>
    struct scene_node_traits;

    template <>
    struct scene_node_traits<SceneNodeState::eShadow> :
        scene_node_shared_traits {
        using type = SceneNodeState;
        static constexpr type value = SceneNodeState::eShadow;

        static constexpr bool distinct_intersect = false;
    };

    template <>
    struct scene_node_traits<SceneNodeState::eLoosy> :
        scene_node_shared_traits {
        using type = SceneNodeState;
        static constexpr type value = SceneNodeState::eLoosy;

        static constexpr bool distinct_intersect = false;
    };

    template <>
    struct scene_node_traits<SceneNodeState::eSpartial> :
        scene_node_shared_traits {
        using type = SceneNodeState;
        static constexpr type value = SceneNodeState::eSpartial;

        static constexpr bool distinct_intersect = true;
    };

    template <>
    struct scene_node_traits<SceneNodeState::eNatural> :
        scene_node_shared_traits {
        using type = SceneNodeState;
        static constexpr type value = SceneNodeState::eNatural;

        static constexpr bool distinct_intersect = true;
    };
}
