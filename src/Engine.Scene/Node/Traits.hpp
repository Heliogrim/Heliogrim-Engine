#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Common/Math/Transform.hpp>

namespace ember::engine::scene {
    template <typename Type_>
    concept HasBoundaryGetter = requires(cref<Type_> obj_) {
        { obj_.getBoundaries() } -> std::same_as<cref<math::Bounding>>;
    };

    template <typename Type_>
    concept HasTransformGetter = requires(cref<Type_> obj_) {
        { obj_.getWorldTransform() } -> std::same_as<cref<math::Transform>>;
    };

    template <class NodeType_, class ElementType_>
    struct scene_traits {
        constexpr static u64 max_childs_per_node = 8ui64;// Octree like
        //constexpr static u64 max_elements_per_leaf = 1ui64;
        //constexpr static u64 max_elements_per_leaf = 16ui64;
        constexpr static u64 max_elements_per_leaf = 128ui64;

        // inline constexpr static size_t node_page_size = 32786;// 32kB Memory Blocks
        constexpr static u64 node_page_size = 1 << 20;// 1MB Memory Blocks
        constexpr static u64 max_nodes_per_page = node_page_size / sizeof(NodeType_);

        // inline constexpr static size_t node_page_size = 32786;// 32kB Memory Blocks
        constexpr static u64 element_page_size = 1 << 20;// 1MB Memory Blocks
        constexpr static u64 max_elements_per_page = element_page_size / sizeof(ptr<ElementType_>);

        constexpr static u64 max_flat_nodes_per_page = max_nodes_per_page / (max_childs_per_node + 1ui64);
        #if ENV_MSVC
        const inline static u64 splice_layer_per_page = _STD floor(
            _STD log2(max_flat_nodes_per_page) / _STD log2(max_childs_per_node)
        );
        #else
        constexpr static u64 splice_layer_per_page = _STD floor(_STD log2(max_flat_nodes_per_page) / _STD log2(max_childs_per_node));
        #endif

        constexpr static float max_boundary_overlap = 0.F;
        constexpr static float min_boundary_overlap = 0.F;

        static_assert(max_elements_per_page >= max_elements_per_leaf,
            "A element page should at least hold enough elements for one node");

        template <HasBoundaryGetter Type = ElementType_>
        inline static cref<math::Bounding> getBounding(const ptr<Type> el_) {
            return el_->getBoundaries();
        }

        template <HasTransformGetter Type = ElementType_>
        inline static cref<math::Transform> getTransform(const ptr<Type> el_) {
            return el_->getTransform();
        }
    };
}
