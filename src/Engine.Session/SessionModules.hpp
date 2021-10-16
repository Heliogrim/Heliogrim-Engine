#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::assets {
    class AssetDatabase;
    class AssetFactory;
}

namespace ember::engine::session {

    class SessionModules {
    public:
        using value_type = SessionModules;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 14.10.2021
         */
        SessionModules() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 14.10.2021
         */
        ~SessionModules() noexcept;

    protected:
        /**
         * Tidies this 
         *
         * @author Julius
         * @date 16.10.2021
         */
        void tidy();

        /**
         * Setups this
         *
         * @author Julius
         * @date 16.10.2021
         */
        void setup();

    public:
        /**
         * Waits until every late constructed module is cleared
         *
         * @author Julius
         * @date 16.10.2021
         */
        void wait() const;

    private:
        ptr<void> _audio;

    public:
        [[nodiscard]] ptr<void> audio() const;

        void setAudio(ptr<void> audio_);

    private:
        ptr<void> _graphics;

    public:
        [[nodiscard]] ptr<void> graphics() const;

        void setGraphics(ptr<void> graphics_);

    private:
        ptr<void> _network;

    public:
        [[nodiscard]] ptr<void> network() const;

        void setNetwork(ptr<void> network_);

    private:
        ptr<void> _physics;

    public:
        [[nodiscard]] ptr<void> physics() const;

        void setPhysics(ptr<void> physics_);

    private:
        ptr<void> _scene;

    public:
        [[nodiscard]] ptr<void> scene() const;

        void setScene(ptr<void> scene_);

    private:
        ptr<engine::assets::AssetDatabase> _assetDatabase;

    public:
        [[nodiscard]] const ptr<engine::assets::AssetDatabase> assetDatabase() const;

    private:
        ptr<engine::assets::AssetFactory> _assetFactory;

    public:
        [[nodiscard]] const ptr<engine::assets::AssetFactory> assetFactory() const;

    private:
        ptr<void> _resourceManager;

    public:
        [[nodiscard]] const ptr<void> resourceManager() const;

    private:
        ptr<void> _ecs;

    public:
        [[nodiscard]] ptr<void> ecs() const;

        void setEcs(ptr<void> ecs_);

    private:
        ptr<void> _scheduler;

    public:
        [[nodiscard]] const ptr<void> scheduler() const;
    };

}
