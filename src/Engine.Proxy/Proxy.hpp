#pragma once
#include "ProxyTypeRegister.hpp"
#include "SignaledStagedCollection.hpp"

namespace ember::engine {

    /**
     * Forward Declaration
     */
    class Session;

    class Proxy {
    public:
        using this_type = Proxy;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 21.11.2021
         */
        Proxy(cref<sptr<Session>> session_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 21.11.2021
         */
        ~Proxy() noexcept;

    private:
        sptr<Session> _session;

    public:
        void setup();

        void schedule();

    private:
        proxy::ProxyTypeRegister _typeRegister;

    private:
        proxy::SignaledStagedCollection _signaled;

    public:
        void signal(cref<component_type_id> typeId_, cref<sptr<proxy::SceneProxiedRef>> proxy_);

    private:
        void batchSignaled();
    };
}
