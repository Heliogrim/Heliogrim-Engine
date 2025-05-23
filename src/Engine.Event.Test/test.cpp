#include "pch.h"

#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Event/TickEvent.hpp>

using namespace hg;

namespace EventModule {
	namespace Event {
		struct TestType01 {};

		struct TestType02 {
			inline static constexpr event_type_id typeId { "TestType02"_typeId };
		};

		struct TestType03 {
			bool canceled() const noexcept {
				return false;
			}

			void cancel() noexcept {}
		};

		struct TestType04 :
			TestType03 {
			inline static constexpr event_type_id typeId { "TestType04"_typeId };
		};

		TEST(Event, TypeCheck) {

			EXPECT_FALSE(IsEvent<TestType01>);
			EXPECT_TRUE(IsEvent<TestType02>);
			EXPECT_FALSE(IsEvent<TestType03>);
			EXPECT_TRUE(IsEvent<TestType04>);

		}

		TEST(StatelessEvent, TypeCheck) {

			EXPECT_FALSE(IsStatelessEvent<TestType01>);
			EXPECT_TRUE(IsStatelessEvent<TestType02>);
			EXPECT_FALSE(IsStatelessEvent<TestType03>);
			EXPECT_FALSE(IsStatelessEvent<TestType04>);

		}

		TEST(StatefulEvent, TypeCheck) {

			EXPECT_FALSE(IsStatefulEvent<TestType01>);
			EXPECT_FALSE(IsStatefulEvent<TestType02>);
			EXPECT_FALSE(IsStatefulEvent<TestType03>);
			EXPECT_TRUE(IsStatefulEvent<TestType04>);

		}
	}

	namespace Emitter {
		TEST(EventEmitter, Listener) {
			//
			auto emitter = StatelessEventEmitter<TickEvent> {};

			//
			auto callback = []([[maybe_unused]] cref<TickEvent> event_) {};
			const auto handle = emitter.on(std::move(callback));

			EXPECT_TRUE(handle);

			//
			emitter.remove(handle);
		}

		TEST(EventEmitter, Emit) {
			//
			auto emitter = StatelessEventEmitter<TickEvent> {};

			//
			const TickEvent event { 1uLL };
			emitter.emit(event);

			//
		}

		TEST(EventEmitter, Cycle) {
			//
			auto emitter = StatelessEventEmitter<TickEvent> {};

			constexpr u64 value { 326326uLL };
			u64 checkValue { 0uLL };

			//
			auto callback = [&](cref<TickEvent> event_) {
				checkValue = event_.tick;
			};
			const auto handle = emitter.on(std::move(callback));

			//
			const TickEvent event { value };

			EXPECT_NE(checkValue, value);
			emitter.emit(event);
			EXPECT_EQ(checkValue, value);

			//
			emitter.remove(handle);
		}
	}
}
