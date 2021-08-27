#pragma once
#include "Graph/SceneGraph.hpp"
#include "Graph/SceneGraphTag.hpp"

namespace ember::engine::scene {
    class Scene {
    public:
        using value_type = Scene;
        using reference_type = ref<Scene>;
        using const_reference_type = ref<const Scene>;

    private:
        /**
         * Singleton Instance
         */
        static ptr<Scene> _instance;

    public:
        /**
         * Gets the static internal stored instance
         *
         * @author Julius
         * @date 09.01.2021
         *
         * @returns A ptr&lt;Scene&gt;
         */
        [[nodiscard]] static ptr<Scene> get() noexcept;

        /**
         * Gets a static internal stored instance of Scene or creates one
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A ptr&lt;Scene&gt;
         */
        [[nodiscard]] static ptr<Scene> make();

        /**
         * Destroys this and flushes static internal stored instance
         *
         * @author Julius
         * @date 08.01.2021
         */
        static void destroy() noexcept;

    private:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 22.03.2021
         */
        Scene() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 16.08.2021
         */
        ~Scene() noexcept;

    public:
        /**
         * Setups this 
         *
         * @author Julius
         * @date 16.08.2021
         */
        void setup();

    private:
        /**
         * Scene Graphs
         */
        vector<uptr<SceneGraph>> _graphs;

    public:
        /**
         * Gets the graphs
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A cref&lt;vector&lt;uptr&lt;SceneGraph&gt;&gt;&gt;
         */
        [[nodiscard]] cref<vector<uptr<SceneGraph>>> graphs() const noexcept;

        /**
         * Gets the graphs
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @returns A ref&lt;vector&lt;uptr&lt;SceneGraph&gt;&gt;&gt;
         */
        [[nodiscard]] ref<vector<uptr<SceneGraph>>> graphs() noexcept;

    private:
        /**
         * The tagged graphs
         */
        vector<_STD pair<SceneGraphTagBase, ptr<SceneGraph>>> _taggedGraphs;

    public:
        /**
         * Gets a SceneGraph via the linked tag_
         *
         * @author Julius
         * @date 27.08.2021
         *
         * @param  tag_ The tag.
         *
         * @returns A pointer to the linked graph or nullptr
         */
        [[nodiscard]] ptr<SceneGraph> getGraph(cref<SceneGraphTagBase> tag_) noexcept;

        /**
         * Gets a SceneGraph via the linked tag_
         *
         * @tparam TagType_ Type of the tag type.
         * @param  tag_ The tag.
         *
         * @returns A pointer to the linked graph or nullptr
         */
        template <HasType TagType_> requires _STD is_convertible_v<TagType_, SceneGraphTagBase>
        [[nodiscard]] ptr<SceneGraph> getGraph(cref<TagType_> tag_) noexcept {
            return getGraph(static_cast<SceneGraphTagBase>(tag_));
        }

        /**
         * Gets or create a SceneGraph and link with given tag_
         *
         * @author Julius
         * @date 27.08.2021
         *
         * @param  tag_ The tag.
         *
         * @returns A pointer to the created SceneGraph
         */
        ptr<SceneGraph> getOrCreateGraph(cref<SceneGraphTagBase> tag_);

        /**
         * Gets or create a SceneGraph and link with given tag_
         *
         * @tparam TagType_ Type of the tag type.
         * @param  tag_ The tag.
         *
         * @returns A pointer to the created SceneGraph
         */
        template <HasType TagType_> requires _STD is_convertible_v<TagType_, SceneGraphTagBase>
        ptr<SceneGraph> getOrCreateGraph(cref<TagType_> tag_) {
            return getOrCreateGraph(static_cast<SceneGraphTagBase>(tag_));
        }
    };
}
