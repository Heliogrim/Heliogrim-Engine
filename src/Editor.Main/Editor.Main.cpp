#ifdef _PROFILING
#include <Engine.Common/Profiling/Profiler.hpp>
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/HeliogrimStatic.hpp>

#include <Editor.Core/EditorEngine.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

using namespace hg;

using SignalShutdownEvent = ::hg::engine::core::SignalShutdownEvent;

int main_impl() {
	#ifdef _PROFILING
	//profiling::Profiler::make().startSession("main");
	//SCOPED_STOPWATCH_V(__main__stopwatch)
	#endif

	/**
	 * Pre-Configure Engine Startup
	 */
	HeliogrimStatic::useEngine<editor::EditorEngine>();
	HeliogrimStatic::useSessionGetter<editor::EditorEngine>(&editor::EditorEngine::getPrimaryGameSession);

	/**
	 * Instantiate Engine
	 */
	Heliogrim::instantiate();
	auto* const engine { Heliogrim::getEngine() };

	/**
	 * Register Shutdown Sequence
	 */
	std::atomic_flag sleepFlag {};
	sleepFlag.test_and_set(std::memory_order::release);

	engine->getEmitter().on<SignalShutdownEvent>(
		[sleepFlag = ptr<std::atomic_flag> { &sleepFlag }](cref<SignalShutdownEvent>) {
			sleepFlag->clear(std::memory_order::relaxed);
			sleepFlag->notify_one();
		}
	);

	/**
	 * Boot Engine
	 */
	engine->preInit();
	engine->init();
	engine->postInit();

	engine->start();

	/**
	 * Set thread asleep
	 */
	sleepFlag.wait(true, std::memory_order::consume);

	/**
	 * Shutdown Engine
	 */
	engine->stop();

	engine->shutdown();
	engine->exit();

	/**
	 * Destroy Engine
	 */
	Heliogrim::destroy();

	#ifdef _PROFILING
	//__main__stopwatch.stop();
	//profiling::Profiler::destroy();
	#endif
	return 0;
}

int main() {
	return main_impl();
}

#if defined(_MSC_VER)

#include <Windows.h>

int WinMain(HINSTANCE hInstance_, HINSTANCE hPrevInstance_, char*, int nsCmd_) {
	return main_impl();
}

#endif
