#include "pch.h"

#include <Engine.Scene/RevScene.hpp>
#include <Engine.Scene/Scene.hpp>

using namespace ember::engine::scene;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace SceneModule {

    namespace Graph { }

    namespace Storage { }

    namespace Rev {
        TEST(RevScene, DefaultGraph) {
            //
            auto* scene = new RevScene();

            //
            EXPECT_NE(scene->renderGraph(), nullptr);
            EXPECT_NE(scene->getWorld(), nullptr);

            //
            delete scene;
        }
    }

    namespace Composite {
        TEST(Head, BodyResolve) {
            //
            auto* scene = new RevScene();

            auto* graph { scene->renderGraph() };

            //
            EXPECT_TRUE(graph->root().nodeId());

            auto& storage { graph->getNodeStorage() };

            const auto& root = graph->root();
            const auto node = root.get(storage.get());

            EXPECT_NE(node, nullptr);

            //
            delete scene;
        }
    }

    #if FALSE
  namespace Node {
        TEST(Container, Default) { }

        TEST(Container, Insert) {
            // Push to Empty
            // Push to Non-Empty
            // Push to Full
            // Insert at Position
        }

        TEST(Container, Iteration) {
            // Begin
            // End
            // Forward Iteration
            // Forward N Iteration
            // Backward Iteration
            // Backward N Iteration
        }

        TEST(Container, Access) {
            // Begin / Front
            // End / Back
            // Random
        }

        TEST(Container, Remove) {
            // Pull from Non-Empty
            // Pull from Empty
            // Remote at Position
        }

        TEST(Types, Casting) {
            // Prepare
            EXPECT_EQ(SceneNodeFactory::get(), nullptr);
            [[maybe_unused]] const auto& scene = Scene::make();
            const auto& factory = SceneNodeFactory::get();

            //
            ptr<SceneNode> base = nullptr;

            //
            base = factory->assembleShadow();
            EXPECT_TRUE(base->isShadowNode());
            EXPECT_FALSE(base->asShadow() == nullptr);
            EXPECT_EQ(base->asLoosy(), nullptr);
            EXPECT_EQ(base->asSpartial(), nullptr);
            EXPECT_EQ(base->asNatural(), nullptr);
            delete base;

            //
            base = factory->assembleLoosy();
            EXPECT_TRUE(base->isLoosyNode());
            EXPECT_FALSE(base->asLoosy() == nullptr);
            EXPECT_EQ(base->asShadow(), nullptr);
            EXPECT_EQ(base->asSpartial(), nullptr);
            EXPECT_EQ(base->asNatural(), nullptr);
            delete base;

            //
            base = factory->assembleSpartial();
            EXPECT_TRUE(base->isSpartialNode());
            EXPECT_FALSE(base->asSpartial() == nullptr);
            EXPECT_EQ(base->asShadow(), nullptr);
            EXPECT_EQ(base->asLoosy(), nullptr);
            EXPECT_EQ(base->asNatural(), nullptr);
            delete base;

            //
            base = factory->assembleNatural();
            EXPECT_TRUE(base->isNaturalNode());
            EXPECT_FALSE(base->asNatural() == nullptr);
            EXPECT_EQ(base->asShadow(), nullptr);
            EXPECT_EQ(base->asLoosy(), nullptr);
            EXPECT_EQ(base->asSpartial(), nullptr);
            delete base;

            // Destroy
            Scene::destroy();
        }

        TEST(Shadow, InsertSingleDirectly) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eShadow>;

            // Prepare
            EXPECT_EQ(SceneNodeFactory::get(), nullptr);
            [[maybe_unused]] const auto& scene = Scene::make();
            const auto& factory = SceneNodeFactory::get();

            //
            auto node = factory->assembleShadow();
            auto child = factory->assembleShadow();

            //
            EXPECT_TRUE(node->isLeaf());
            EXPECT_TRUE(node->depth() == 0);

            EXPECT_TRUE(node->push(_STD move(*child)));

            EXPECT_FALSE(node->isLeaf());
            EXPECT_TRUE(node->depth() == 1);

            //
            delete node;

            // Destroy
            Scene::destroy();
        }

        TEST(Shadow, ContainsSingleDirectly) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eShadow>;

            // Prepare
            EXPECT_EQ(SceneNodeFactory::get(), nullptr);
            [[maybe_unused]] const auto& scene = Scene::make();
            const auto& factory = SceneNodeFactory::get();

            //
            auto node = factory->assembleShadow();
            auto child = factory->assembleShadow();

            //
            node->push(_STD move(*child));

            //
            EXPECT_TRUE(node->contains(*child));

            //
            delete node;

            // Destroy
            Scene::destroy();
        }

        TEST(Shadow, RemoveLeafDirectly) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eShadow>;

            // Prepare
            EXPECT_EQ(SceneNodeFactory::get(), nullptr);
            [[maybe_unused]] const auto& scene = Scene::make();
            const auto& factory = SceneNodeFactory::get();

            //
            auto node = factory->assembleShadow();
            auto child = factory->assembleShadow();

            //
            const auto nodeId = child->nodeId();
            node->push(_STD move(*child));

            //
            EXPECT_FALSE(node->isLeaf());
            EXPECT_TRUE(node->depth() == 1);

            auto result = node->pull(nodeId);
            EXPECT_TRUE(result);
            EXPECT_TRUE(result.value != nullptr);

            EXPECT_TRUE(node->isLeaf());
            EXPECT_TRUE(node->depth() == 0);

            //
            delete result.value;
            result.value = nullptr;

            delete node;

            // Destroy
            Scene::destroy();
        }

        TEST(Shadow, InsertMaxPerLayer) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eShadow>;

            // Prepare
            EXPECT_EQ(SceneNodeFactory::get(), nullptr);
            [[maybe_unused]] const auto& scene = Scene::make();
            const auto& factory = SceneNodeFactory::get();

            //
            auto node = factory->assembleShadow();

            //
            EXPECT_TRUE(node->isLeaf());
            EXPECT_TRUE(node->depth() == 0);

            for (shared_type_trait::container_size_type idx = 0; idx < shared_type_trait::max_nodes_per_layer; ++idx) {
                auto child = factory->assembleShadow();
                EXPECT_TRUE(node->push(_STD move(*child)));
            }

            EXPECT_FALSE(node->isLeaf());
            EXPECT_TRUE(node->depth() == 1);
            EXPECT_TRUE(node->children().size() == shared_type_trait::max_nodes_per_layer);

            //
            delete node;

            // Destroy
            Scene::destroy();
        }

        TEST(Shadow, InsertOverMaxPerLayer) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eShadow>;

            // Prepare
            EXPECT_EQ(SceneNodeFactory::get(), nullptr);
            [[maybe_unused]] const auto& scene = Scene::make();
            const auto& factory = SceneNodeFactory::get();

            //
            auto node = factory->assembleShadow();

            //
            EXPECT_TRUE(node->isLeaf());

            constexpr u64 mnpl = shared_type_trait::max_nodes_per_layer;
            constexpr u64 count = mnpl * mnpl + 1ui64;
            for (u64 idx = 0; idx < count; ++idx) {
                auto child = factory->assembleShadow();
                EXPECT_TRUE(node->push(_STD move(*child)));
            }

            EXPECT_FALSE(node->isLeaf());
            EXPECT_TRUE(node->depth() == 2);
            EXPECT_TRUE(node->size() == count + /* + Base Parent Node */1ui64);
            EXPECT_TRUE(node->children().size() == shared_type_trait::max_nodes_per_layer);

            //
            delete node;

            // Destroy
            Scene::destroy();
        }

        TEST(Shadow, RemoveNonLeafChild) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eShadow>;

            // Prepare
            EXPECT_EQ(SceneNodeFactory::get(), nullptr);
            [[maybe_unused]] const auto& scene = Scene::make();
            const auto& factory = SceneNodeFactory::get();

            //
            auto node = factory->assembleShadow();

            //
            constexpr u64 mnpl = shared_type_trait::max_nodes_per_layer;
            constexpr u64 count = /* Child for Layer 3 */ mnpl * mnpl * mnpl +
                /* Child for Layer 2*/ mnpl * mnpl +
                /* Child for Layer 1*/ mnpl;

            for (u64 idx = 0; idx < count; ++idx) {
                const auto child = factory->assembleShadow();
                node->push(_STD move(*child));
            }

            // Find scenario matching node
            ptr<SceneNode> testNode = nullptr;

            {
                SceneNode::container_type::iterator iter = node->children().begin();
                const SceneNode::container_type::iterator end = node->children().end();

                for (; iter != end; ++iter) {
                    if (!iter->isLeaf()) {
                        testNode = iter.pointer();
                        break;
                    }
                }

                EXPECT_FALSE(node->isLeaf());
                EXPECT_TRUE(testNode != nullptr);
                EXPECT_FALSE(testNode->isLeaf());
            }

            //
            EXPECT_EQ(node->children().size(), shared_type_trait::max_nodes_per_layer);
            EXPECT_EQ(node->size(), count + 1ui64);

            auto result = node->pull(*testNode);
            EXPECT_TRUE(result);
            EXPECT_TRUE(result.value != nullptr);

            EXPECT_EQ(node->children().size(), shared_type_trait::max_nodes_per_layer);
            EXPECT_EQ(node->size(), count + 1ui64 - 1ui64);

            //
            delete result.value;
            result.value = nullptr;

            delete node;

            // Destroy
            Scene::destroy();

        }

        TEST(Shadow, Capture) { }

        TEST(Loosy, InsertSingle) { }

        TEST(Loosy, InsertMaxPerLayer) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eLoosy>;
        }

        TEST(Loosy, Capture) { }

        TEST(Spartial, InsertSingle) { }

        TEST(Spartial, InsertMaxPerLayer) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eSpartial>;
        }

        TEST(Spartial, Capture) { }

        TEST(Natural, InsertSingle) { }

        TEST(Natural, InsertMaxPerLayer) {

            using shared_type_trait = scene_node_shared_traits;
            using type_trait = scene_node_traits<SceneNodeState::eNatural>;
        }

        TEST(Natural, Capture) { }
    }
    #endif

    #if FALSE
  namespace Graph {
        TEST(Main, InsertSingle) { }

        TEST(Main, ContainsSingle) { }

        TEST(Main, RemoveSingle) { }

        TEST(Main, InsertMany) { }

        TEST(Main, RemoveMany) { }

        TEST(Main, TraverseSingle) {

            uint64_t counter = 0;

            const auto consumer = [&counter](cref<SceneNode>) {
                return ++counter <= 1;
            };

            //
            const auto scene = Scene::make();
            const auto& graph = scene->graph();

            // TODO: Insert element to graph
            EXPECT_EQ(counter, 0);
            graph.traversal(consumer);
            EXPECT_EQ(counter, 1);

            //
            Scene::destroy();
        }

        TEST(Main, TraverseFull) {
            uint64_t counter = 0;
            constexpr uint64_t elements = 8 * 8;

            const auto consumer = [&counter](cref<SceneNode>) {
                ++counter;
                return true;
            };

            //
            const auto scene = Scene::make();
            const auto factory = SceneNodeFactory::get();
            const auto& graph = scene->graph();

            for (auto idx = 1; idx < elements; ++idx) {
                const auto node = factory->assembleShadow();
                graph.root()->push(_STD move(*node));
            }

            EXPECT_EQ(counter, 0);
            graph.traversal(consumer);
            EXPECT_EQ(counter, elements);

            //
            Scene::destroy();
        }

        TEST(Sub, Traverse) {
            uint64_t counter = 0;

            const auto consumer = [&counter](cref<SceneNode>) {
                return ++counter <= 1;
            };

            //
            const auto scene = Scene::make();
            const auto& graph = scene->graph();

            const auto& subroot = graph.root();
            const SceneNodeGraph subgraph { { subroot } };

            // TODO: Insert element to graph
            EXPECT_EQ(counter, 0);
            subgraph.traverse(consumer);
            EXPECT_EQ(counter, 1);

            //
            Scene::destroy();
        }
    }
    #endif
}
