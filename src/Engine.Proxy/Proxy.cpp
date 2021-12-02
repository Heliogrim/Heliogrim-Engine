#include "Proxy.hpp"

#include <Engine.Session/Session.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "Engine.Event/ShutdownEvent.hpp"

using namespace ember::engine;
using namespace ember;

/**
 * TODO: Temporary
 */
static _STD atomic_flag proxy_shutdown {};

Proxy::Proxy(cref<sptr<Session>> session_) noexcept :
    _session(session_),
    _signaled() {}

Proxy::~Proxy() noexcept = default;

void Proxy::setup() {}

void Proxy::schedule() {

    /**
     * TODO: Temporary
     */
    _session->emitter().on<ShutdownEvent>([](cref<ShutdownEvent>) {
        proxy_shutdown.clear(_STD memory_order_release);
    });
    proxy_shutdown.test_and_set(_STD memory_order_release);

    auto* shd { _session->modules().scheduler() };
    shd->exec(make_repetitive_task([this]() {
            if (proxy_shutdown.test(_STD memory_order_consume)) {
                return false;
            }

            batchSignaled();
            return true;
        },
        scheduler::task::TaskMask::eNormal,
        scheduler::ScheduleStageBarriers::ePublishStrong,
        scheduler::ScheduleStageBarriers::ePublishStrong
    ));

}

void Proxy::signal(cref<component_type_id> typeId_, cref<sptr<proxy::SceneProxiedRef>> proxy_) {
    _signaled.signal(typeId_, proxy_);
}

void Proxy::batchSignaled() {

    /**
     * Get snapshot of current signaled proxies
     */
    auto signaled = _signaled.exchange();

    for (auto& entry : signaled) {

        // TODO:
        auto fnc = _typeRegister.get(entry.typeId);

        if (fnc != nullptr) {
            fnc(entry.proxied.get());
        }
    }

}
