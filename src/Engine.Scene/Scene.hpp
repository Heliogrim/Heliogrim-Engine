#pragma once
#include "./Graph/SceneGraph.hpp"

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

    private:
        /**
         * Sets up the graphics scene
         *
         * @author Julius
         * @date 16.08.2021
         */
        void setupGfxScene();

        /**
         * Sets up the physics scene
         *
         * @author Julius
         * @date 16.08.2021
         */
        void setupPfxScene();

        /**
         * Sets up the sounds scene
         *
         * @author Julius
         * @date 16.08.2021
         */
        void setupSfxScene();

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
    };
}
