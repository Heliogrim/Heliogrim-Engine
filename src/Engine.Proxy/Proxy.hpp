#pragma once

#include <Engine.Session/Session.hpp>

#include "ProxyTypeRegister.hpp"
#include "SignaledStagedCollection.hpp"

namespace ember::engine {

    class Proxy {
    public:
        using this_type = Proxy;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 21.11.2021
         *
         * @param session_ (Optional) The session related.
         */
        Proxy(cref<sptr<Session>> session_ = Session::get()) noexcept;

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
