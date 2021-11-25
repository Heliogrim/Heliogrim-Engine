#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine {
    class Proxy;
}

namespace ember::engine::assets {
    class AssetDatabase;
    class AssetFactory;
    class AssetSeeder;
}

namespace ember::engine::acs {
    class Registry;
}

namespace ember::engine::scene {
    class Scene;
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
         * Starts this
         *
         * @author Julius
         * @date 20.10.2021
         */
        void start();

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
        ptr<engine::Proxy> _proxy;

    public:
        [[nodiscard]] ptr<engine::Proxy> proxy() const;

        void setProxy(ptr<engine::Proxy> proxy_);

    private:
        ptr<scene::Scene> _scene;

    public:
        [[nodiscard]] ptr<scene::Scene> scene() const;

        void setScene(ptr<scene::Scene> scene_);

    private:
        ptr<engine::assets::AssetDatabase> _assetDatabase;

    public:
        [[nodiscard]] const ptr<engine::assets::AssetDatabase> assetDatabase() const;

    private:
        ptr<engine::assets::AssetFactory> _assetFactory;

    public:
        [[nodiscard]] const ptr<engine::assets::AssetFactory> assetFactory() const;

    private:
        ptr<engine::assets::AssetSeeder> _assetSeeder;

    public:
        [[nodiscard]] const ptr<engine::assets::AssetSeeder> assetSeeder() const;

    private:
        ptr<void> _resourceManager;

    public:
        [[nodiscard]] const ptr<void> resourceManager() const;

    private:
        ptr<acs::Registry> _registry;

    public:
        [[nodiscard]] const ptr<acs::Registry> registry() const;

        void setRegistry(ptr<acs::Registry> registry_);

    private:
        ptr<void> _scheduler;

    public:
        [[nodiscard]] const ptr<void> scheduler() const;
    };

}
