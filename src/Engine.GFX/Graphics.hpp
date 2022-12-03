#pragma once

#include <Engine.Session/Session.hpp>

#include "Application/Application.hpp"
#include "Command/CommandQueue.hpp"
#include "Device/Device.hpp"
#include "Renderer/__fwd.hpp"
#include "Cache/__fwd.hpp"
#include "Surface/Surface.hpp"
#include "Swapchain/Swapchain.hpp"

#include <Engine.Common/Math/Vector.hpp>

#include "Engine.Common/Collection/AssociativeKey.hpp"
#include "Engine.Common/Collection/RobinMap.hpp"

/**
 * Forward Declaration
 */
namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::gfx {
    class RenderTarget;
}

namespace ember::engine::gfx::scene {
    class RenderSceneManager;
    class SceneView;
}

namespace ember::engine::gfx::loader {
    class GeometryLoader;
    class TextureLoader;
}

namespace ember::engine {

    class Graphics {
    public:
        using this_type = Graphics;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @param session_ (Optional) The session.
         */
        Graphics(cref<sptr<Session>> session_ = Session::get()) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        ~Graphics();

    public:
        /**
         * Setups this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        void setup();

        /**
         * Schedules this module
         *
         * @author Julius
         * @date 19.12.2020
         */
        void schedule();

    private:
        /**
         * Tidy this up
         *
         * @author Julius
         * @date 13.09.2021
         */
        void tidy();

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
         * @returns A ptr<Session>
         */
        [[nodiscard]] sptr<Session> session() const noexcept;

    private:
        /**
         * The internal state of scheduler
         */
        _STD atomic_uint8_t _scheduled { 0 };

    private:
        gfx::Application _application;
        gfx::Surface _surface;

    private:
        /**
         * Device
         */
        sptr<gfx::Device> _device;

    public:
        /**
         * Gets current device
         *
         * @author Julius
         * @date 29.01.2021
         *
         * @returns The current device.
         */
        [[nodiscard]] sptr<gfx::Device> getCurrentDevice() const noexcept;

    private:
        /**
         * Swapchain
         */
        ptr<gfx::Swapchain> _swapchain;

        #if TRUE
    public:
        ptr<gfx::Swapchain> _secondarySwapchain;
        #endif

    public:
        /**
         * Gets current swapchain
         *
         * @author Julius
         * @date 29.01.2021
         *
         * @returns The current swapchain.
         */
        [[nodiscard]] ptr<gfx::Swapchain> getCurrentSwapchain() const noexcept;

    private:
        /**
         * Compute Queue
         */
        ptr<gfx::CommandQueue> _computeQueue;

    public:
        /**
         * Gets compute queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns The compute queue.
         */
        [[nodiscard]] gfx::CommandQueue::reference_type getComputeQueue() const noexcept;

    private:
        /**
         * Graphics
         */
        ptr<gfx::CommandQueue> _graphicsQueue;

    public:
        /**
         * Gets graphics queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns The graphics queue.
         */
        [[nodiscard]] gfx::CommandQueue::reference_type getGraphicsQueue() const noexcept;

    private:
        /**
         * Transfer Queue
         */
        ptr<gfx::CommandQueue> _transferQueue;

    public:
        /**
         * Gets transfer queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns The transfer queue.
         */
        [[nodiscard]] gfx::CommandQueue::reference_type getTransferQueue() const noexcept;

    private:
        uptr<gfx::cache::GlobalCacheCtrl> _cacheCtrl;

    public:
        [[nodiscard]] const non_owning_rptr<gfx::cache::GlobalCacheCtrl> cacheCtrl() const noexcept;

    private:
        /**
         *
         */
        RobinMap<AssocKey<string>, sptr<gfx::render::Renderer>> _cachedRenderer;

    public:
        [[nodiscard]] sptr<gfx::render::Renderer> getRenderer(cref<AssocKey<string>> key_) const;

        [[nodiscard]] sptr<gfx::render::Renderer> getRenderer(
            cref<AssocKey<string>> key_,
            _STD nothrow_t) const noexcept;

        [[nodiscard]] bool hasRenderer(cref<AssocKey<string>> key_);

        [[nodiscard, deprecated("Might create memory leak and not-freeable resources")]] bool removeRenderer(
            cref<AssocKey<string>> key_);

    private:
        ptr<gfx::scene::RenderSceneManager> _sceneManager;

    public:
        [[nodiscard]] const non_owning_rptr<gfx::scene::RenderSceneManager> getSceneManager() const noexcept;

    private:
        void tick();

        void invokeRenderTarget(cref<sptr<gfx::RenderTarget>> target_) const;

        #pragma region Ember Graphics

    private:
        void reschedule();

    private:
        Vector<ptr<gfx::loader::GeometryLoader>> _geometryLoader;
        Vector<ptr<gfx::loader::TextureLoader>> _textureLoader;

    public:
        void registerLoader();

    private:
        Vector<ptr<void>> _importer;

    public:
        void registerImporter();

        #pragma endregion

        #if TRUE
    public:
        void __tmp__resize(cref<math::uivec2> extent_);
        #endif
    };
}
