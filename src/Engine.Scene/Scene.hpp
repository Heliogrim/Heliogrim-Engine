#pragma once

#include <Engine.Session/Session.hpp>

#include "Graph/SceneGraph.hpp"
#include "Graph/SceneGraphTag.hpp"

namespace ember::engine::scene {
    class Scene {
    public:
        using value_type = Scene;
        using reference_type = ref<Scene>;
        using const_reference_type = ref<const Scene>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 22.03.2021
         *
         * @param  session_ The session.
         */
        Scene(cref<sptr<Session>> session_) noexcept;

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
         * The session this module is associated with
         */
        sptr<Session> _session;

    public:
        /**
         * Gets the session this module is associated with
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A sptr<Session>
         */
        [[nodiscard]] sptr<Session> session() const noexcept;

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
         * Gets a SceneGraph via the linked tag
         *
         * @author Julius
         * @date 01.11.2021
         *
         * @tparam TagType_ Type of the tag type and source of substituted tag id.
         *
         * @returns A pointer to the linked graph or nullptr
         */
        template <HasType TagType_> requires _STD is_nothrow_convertible_v<TagType_, SceneGraphTagBase> &&
            _STD is_nothrow_default_constructible_v<TagType_>
        [[nodiscard]] _Success_(return != nullptr) ptr<SceneGraph> getGraph() noexcept {
            return getGraph(static_cast<SceneGraphTagBase>(TagType_ {}));
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

        /**
         * Get or create a SceneGraph and link with given tag
         *
         * @author Julius
         * @date 02.11.2021
         *
         * @tparam TagType_ Type of the tag type and source of substituted tag id.
         *
         * @returns A pointer to the linked graph or nullptr
         */
        template <HasType TagType_> requires _STD is_nothrow_convertible_v<TagType_, SceneGraphTagBase> &&
            _STD is_nothrow_default_constructible_v<TagType_>
        _Success_(return != nullptr) ptr<SceneGraph> getOrCreateGraph() {
            return getOrCreateGraph(static_cast<SceneGraphTagBase>(TagType_ {}));
        }
    };
}
