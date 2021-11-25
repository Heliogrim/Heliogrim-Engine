#pragma once
#include <Ember/ActorComponent.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "ProxiedScenePayload.hpp"
#include "SceneProxy.hpp"

namespace ember::engine::proxy {

    /**
     * A reference between SceneComponent, SceneElement and SceneProxy
     *
     *  The lifetime of SceneElement depends on lifetime of SceneProxiedRef.
     *  SceneProxiedRef should only be used with `managed` wrapper of common library.
     *
     *  [On Destroy]
     *      If SceneProxiedRef gets destroyed, it will mask the SceneElement as deleted.
     *      SceneElement's ModelState will be destroyed by queued commands after render-threading finished frame and resorted SceneGraph.
     *      SceneElement which is marked as deleted, will be skipped within SceneGraph.
     *      SceneNode related to SceneElement will be replaced or resorted after currently running render-threading finished frame.
     *
     *  [On Owner Change]
     *      If SceneProxy does not change, ModelState will be reused, otherwise new ModelState will be requested and allocated.
     *      The ModelState will be push to SceneElement.
     *      The ModelBatch will be flushed to recreate Commands.
     *
     *  [On Creation]
     *      A Managed instance of SceneProxiedRef will be created and referenced.
     *      A ModelState will be requested and allocated, defined by given SceneProxy.
     *      A SceneElement will be requested.
     *      The ModelState and ModelBatch data will be linked to SceneElement.
     *      SceneElement will be pushed and sorted into SceneGraph.
     *
     * @author Julius
     * @date 08.01.2021
     */
    class SceneProxiedRef final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.01.2021
         */
        SceneProxiedRef() = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 08.01.2021
         */
        ~SceneProxiedRef() noexcept;

    private:
        /**
         * Owner
         */
        non_owning_rptr<ActorComponent> _owner;

    public:
        /**
         * Gets the owner
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A pointer to the owning scene component
         */
        [[nodiscard]] non_owning_rptr<ActorComponent> owner() const noexcept;

        /**
         * Gets the owner
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @returns A pointer to the owning scene component
         */
        [[nodiscard]] non_owning_rptr<ActorComponent> owner() noexcept;

    private:
        /**
         * Scene Proxy
         */
        uptr<SceneProxy> _proxy;

    public:
        /**
         * Get a pointer to the SceneProxy
         *
         * @author Julius
         * @date 07.01.2021
         *
         * @returns A ptr&lt;SceneProxy&gt;
         */
        [[nodiscard]] ptr<SceneProxy> proxy() const noexcept;

    private:
        /**
         * Scene Payload
         */
        OwningProxiedScenePayload _payload;

    public:
        /**
         * Get a pointer to the payload
         *
         * @author Julius
         * @date 07.01.2021
         *
         * @returns A ptr&lt;SceneElement&gt;
         */
        [[nodiscard]] cref<OwningProxiedScenePayload> payload() const noexcept;

        [[nodiscard, deprecated]] ref<OwningProxiedScenePayload> payload() noexcept;

    private:
        /**
         * Constructor
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @param   owner_ The owner.
         * @param   proxy_ The proxy.
         * @param   payload_ The scene payload.
         */
        SceneProxiedRef(_In_ non_owning_rptr<ActorComponent> owner_,
            _Inout_ mref<uptr<SceneProxy>> proxy_,
            _Inout_ mref<OwningProxiedScenePayload> payload_) noexcept;

    public:
        /**
         * Managed Construction call to SceneProxiedRef
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @param  owner_ The owner.
         *
         * @returns A managed&lt;SceneProxiedRef&gt;
         */
        static managed<SceneProxiedRef> make_proxied_ref(non_owning_rptr<ActorComponent> owner_);

        /**
         * Managed Construction call to SceneProxiedRef without initial state and owner
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @param  proxy_ The proxy.
         *
         * @returns A managed&lt;SceneProxiedRef&gt;
         */
        static managed<SceneProxiedRef> make_proxied_ref(_Inout_ mref<uptr<SceneProxy>> proxy_);

        /**
         * Managed Construction call to SceneProxiedRef without substitution of SceneProxy by SceneComponent
         *
         * @author Julius
         * @date 08.01.2021
         *
         * @param  proxy_ The proxy.
         * @param  owner_ The owner.
         *
         * @returns A managed&lt;SceneProxiedRef&gt;
         */
        static managed<SceneProxiedRef> make_proxied_ref(_Inout_ mref<uptr<SceneProxy>> proxy_,
            non_owning_rptr<ActorComponent> owner_);
    };
}
