#include "pch.h"

/**/

#include <Engine.Async/Future/Async.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Waitable/JoinCounter.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	TEST(RuntimeTest, Default) {
		structuredRuntimeTest(nullptr);
	}

	TEST(RuntimeTest, SimpleTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				std::atomic_flag signal {};
				auto task {
					task::make_task(
						[&signal]() {
							signal.test_and_set();
						}
					)
				};

				/**
				 *
				 */
				scheduler_->exec(std::move(task));

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!signal.test() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(signal.test());
			}
		);
	}

	TEST(RuntimeTest, SimplePromiseTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> scheduler_) {

				/**
				 *
				 */
				auto fut { ::hg::async([] {}) };

				/**
				 *
				 */
				u32 retry { 0uL };
				while (!fut.isReady() && retry < 200) {
					thread::self::sleepFor(5);
					++retry;
				}

				/**
				 *
				 */
				EXPECT_TRUE(fut.isReady());
			}
		);
	}

	TEST(RuntimeTest, PrioritySorting) {

		std::chrono::high_resolution_clock::time_point minorTimestamp {};
		std::chrono::high_resolution_clock::time_point majorTimestamp {};

		// one worker: with more, an idle worker pops the eLower task before the eHigher one is queued
		structuredRuntimeTest(
			1uL,
			[&minorTimestamp, &majorTimestamp](nmpt<engine::Scheduler> scheduler_) {

				JoinCounter done { 3u };

				auto minorTask = engine::scheduler::task::make_task(
					[&minorTimestamp, &done]() {
						minorTimestamp = std::chrono::high_resolution_clock::now();
						done.done();
					},
					task::TaskMask::eLower
				);

				auto majorTask = engine::scheduler::task::make_task(
					[&majorTimestamp, &done]() {
						majorTimestamp = std::chrono::high_resolution_clock::now();
						done.done();
					},
					task::TaskMask::eHigher
				);

				/**/

				auto dispatchTask = engine::scheduler::task::make_task(
					[&minorTask, &majorTask, &scheduler_, &done]() {
						scheduler_->exec(std::move(minorTask));
						scheduler_->exec(std::move(majorTask));
						done.done();
					}
				);

				scheduler_->exec(std::move(dispatchTask));

				/**
				 *
				 */
				done.wait();
			}
		);

		EXPECT_LT(majorTimestamp, minorTimestamp);
	}
}
