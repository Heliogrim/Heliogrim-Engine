#pragma once

#include <Engine.Core/Engine.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#ifndef USE_MAIN_THREAD_INJECTION
#define USE_MAIN_THREAD_INJECTION FALSE
#endif

#ifndef PREVENT_MAIN_AUTO_SLEEP
#define PREVENT_MAIN_AUTO_SLEEP FALSE
#endif

using Engine = ::hg::engine::Engine;

#if USE_MAIN_THREAD_INJECTION

extern void beforeRoutine();

extern void onEngineCreate(const hg::non_owning_rptr<Engine> engine_);

extern void onEnginePreInit(const hg::non_owning_rptr<Engine> engine_);

extern void onEngineInit(const hg::non_owning_rptr<Engine> engine_);

extern void onEnginePostInit(const hg::non_owning_rptr<Engine> engine_);

extern void onEngineStart(const hg::non_owning_rptr<Engine> engine_);

#if not PREVENT_MAIN_AUTO_SLEEP

/**
 * Invoked when engine completed startup sequence
 *
 * @note When this function returns, the main thread used for the invocation
 *  will be put asleep until the engine get's a shutdown signal, which is
 *  handled by the main thread.
 */
extern void onEngineRunning(const hg::non_owning_rptr<Engine> engine_);

#else

/**
 * Invoked when engine completed startup sequence
 *
 * @note When this function returns, the main thread will automatically
 *  stop / shutdown the whole engine by completing the runtime routine.
 *  If you won't use the auto sleep but intend to use this helper routine,
 *  please take care that you block the returning of this function until
 *  the shutdown should arrive.
 *
 * @note This function call will force the responsibility on you to handle,
 *  the SignalShutdownEvent emitted by the engine event system.
 */
extern void onEngineRunning(const hg::non_owning_rptr<Engine> engine_);

#endif

extern void onEngineStop(const hg::non_owning_rptr<Engine> engine_);

extern void onEngineShutdown(const hg::non_owning_rptr<Engine> engine_);

extern void onEngineExit(const hg::non_owning_rptr<Engine> engine_);

extern void afterRoutine();

#endif
