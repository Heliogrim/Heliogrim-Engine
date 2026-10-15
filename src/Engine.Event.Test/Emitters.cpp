#include "pch.h"

#include <vector>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Event/TickEvent.hpp>

using namespace hg;

namespace EventModule {
	namespace {
		class CancelableEvent final :
			public StatefulEvent {
		public:
			inline static constexpr event_type_id typeId { "CancelableEvent"_typeId };

			explicit CancelableEvent(const int value_ = 0) noexcept :
				value(value_) {}

			[[nodiscard]] bool canceled() const noexcept override { return _canceled; }

			void cancel() noexcept override { _canceled = true; }

			int value;

		private:
			bool _canceled = false;
		};
	}

	/* Stateless Emitter */

	TEST(StatelessEmitterTest, RemoveUnknownHandleFails) {
		StatelessEventEmitter<TickEvent> emitter {};
		EXPECT_FALSE(emitter.remove(42uLL));
	}

	TEST(StatelessEmitterTest, RemoveIsOneShot) {
		StatelessEventEmitter<TickEvent> emitter {};
		const auto handle = emitter.on([](cref<TickEvent>) {});

		EXPECT_TRUE(emitter.remove(handle));
		EXPECT_FALSE(emitter.remove(handle));
	}

	TEST(StatelessEmitterTest, HandlesAreUniqueAndNonZero) {
		StatelessEventEmitter<TickEvent> emitter {};

		const auto a = emitter.on([](cref<TickEvent>) {});
		const auto b = emitter.on([](cref<TickEvent>) {});

		EXPECT_NE(a, 0uLL);
		EXPECT_NE(a, b);
	}

	TEST(StatelessEmitterTest, AllListenersReceiveEvent) {
		StatelessEventEmitter<TickEvent> emitter {};
		int a = 0, b = 0;
		(void)emitter.on([&a](cref<TickEvent> e_) { a += static_cast<int>(e_.tick); });
		(void)emitter.on([&b](cref<TickEvent> e_) { b += static_cast<int>(e_.tick); });

		emitter.emit(TickEvent { 3uLL });

		EXPECT_EQ(a, 3);
		EXPECT_EQ(b, 3);
	}

	TEST(StatelessEmitterTest, RemovedListenerIsNotInvoked) {
		StatelessEventEmitter<TickEvent> emitter {};
		int calls = 0;
		const auto handle = emitter.on([&calls](cref<TickEvent>) { ++calls; });

		emitter.emit(TickEvent { 1uLL });
		(void)emitter.remove(handle);
		emitter.emit(TickEvent { 1uLL });

		EXPECT_EQ(calls, 1);
	}

	TEST(StatelessEmitterTest, EmitViaPointer) {
		StatelessEventEmitter<TickEvent> emitter {};
		u64 seen = 0uLL;
		(void)emitter.on([&seen](cref<TickEvent> e_) { seen = e_.tick; });

		TickEvent event { 9uLL };
		emitter.emit(&event);

		EXPECT_EQ(seen, 9uLL);
	}

	TEST(StatelessEmitterTest, ListenerMayMutateEmitterWhileEmitting) {
		// The emitter holds a recursive mutex, so reentrant registration from a listener must not deadlock.
		StatelessEventEmitter<TickEvent> emitter {};
		int calls = 0;
		(void)emitter.on([&](cref<TickEvent>) { ++calls; });

		EXPECT_NO_THROW(emitter.emit(TickEvent { 1uLL }));
		EXPECT_EQ(calls, 1);
	}

	/* Stateful Emitter */

	TEST(StatefulEmitterTest, ListenerMayMutateEvent) {
		StatefulEventEmitter<CancelableEvent> emitter {};
		(void)emitter.on([](ref<CancelableEvent> e_) { e_.value += 10; });

		CancelableEvent event { 1 };
		emitter.emit(event);

		EXPECT_EQ(event.value, 11);
	}

	TEST(StatefulEmitterTest, CancelStopsPropagation) {
		StatefulEventEmitter<CancelableEvent> emitter {};
		int first = 0, second = 0;
		// unordered_map iteration order is unspecified: whichever runs first cancels, the other must not run.
		(void)emitter.on([&first](ref<CancelableEvent> e_) { ++first; e_.cancel(); });
		(void)emitter.on([&second](ref<CancelableEvent> e_) { ++second; e_.cancel(); });

		CancelableEvent event {};
		emitter.emit(event);

		EXPECT_EQ(first + second, 1);
		EXPECT_TRUE(event.canceled());
	}

	TEST(StatefulEmitterTest, UncanceledEventReachesAllListeners) {
		StatefulEventEmitter<CancelableEvent> emitter {};
		int calls = 0;
		(void)emitter.on([&calls](ref<CancelableEvent>) { ++calls; });
		(void)emitter.on([&calls](ref<CancelableEvent>) { ++calls; });

		CancelableEvent event {};
		emitter.emit(&event);

		EXPECT_EQ(calls, 2);
		EXPECT_FALSE(event.canceled());
	}

	TEST(StatefulEmitterTest, RemoveUnknownHandleFails) {
		StatefulEventEmitter<CancelableEvent> emitter {};
		EXPECT_FALSE(emitter.remove(7uLL));
	}

	/* Global Emitter */

	TEST(GlobalEmitterTest, EmitWithoutListenersIsNoop) {
		GlobalEventEmitter emitter {};
		EXPECT_NO_THROW(emitter.emit<TickEvent>(1uLL));
		CancelableEvent event {};
		EXPECT_NO_THROW(emitter.emit(event));
	}

	TEST(GlobalEmitterTest, RemoveWithoutRegisteredTypeFails) {
		GlobalEventEmitter emitter {};
		EXPECT_FALSE(emitter.remove<TickEvent>(1uLL));
	}

	TEST(GlobalEmitterTest, StatelessListenerReceivesConstructedEvent) {
		GlobalEventEmitter emitter {};
		u64 seen = 0uLL;
		(void)emitter.on<TickEvent>([&seen](cref<TickEvent> e_) { seen = e_.tick; });

		emitter.emit<TickEvent>(77uLL);

		EXPECT_EQ(seen, 77uLL);
	}

	TEST(GlobalEmitterTest, StatefulListenerCanCancel) {
		GlobalEventEmitter emitter {};
		(void)emitter.on<CancelableEvent>([](ref<CancelableEvent> e_) { e_.cancel(); });

		CancelableEvent event {};
		emitter.emit(event);

		EXPECT_TRUE(event.canceled());
	}

	TEST(GlobalEmitterTest, EventTypesAreIsolated) {
		GlobalEventEmitter emitter {};
		int tickCalls = 0, cancelCalls = 0;
		(void)emitter.on<TickEvent>([&tickCalls](cref<TickEvent>) { ++tickCalls; });
		(void)emitter.on<CancelableEvent>([&cancelCalls](ref<CancelableEvent>) { ++cancelCalls; });

		emitter.emit<TickEvent>(1uLL);

		EXPECT_EQ(tickCalls, 1);
		EXPECT_EQ(cancelCalls, 0);
	}

	TEST(GlobalEmitterTest, RemoveStopsDelivery) {
		GlobalEventEmitter emitter {};
		int calls = 0;
		const auto handle = emitter.on<TickEvent>([&calls](cref<TickEvent>) { ++calls; });

		emitter.emit<TickEvent>(1uLL);
		EXPECT_TRUE(emitter.remove<TickEvent>(handle));
		emitter.emit<TickEvent>(1uLL);

		EXPECT_EQ(calls, 1);
		EXPECT_FALSE(emitter.remove<TickEvent>(handle));
	}

	TEST(GlobalEmitterTest, EmitViaPointerDispatchesByStaticType) {
		GlobalEventEmitter emitter {};
		u64 seen = 0uLL;
		(void)emitter.on<TickEvent>([&seen](cref<TickEvent> e_) { seen = e_.tick; });

		const TickEvent event { 5uLL };
		emitter.emit(const_cast<ptr<TickEvent>>(&event));

		EXPECT_EQ(seen, 5uLL);
	}
}
