#include "pch.h"

#include <Engine.Scheduler/Scheduler.hpp>

using namespace ember::engine::scheduler;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

TEST(SchedulerModule, Singleton) {
    //
    EXPECT_EQ(&Scheduler::get(), nullptr);

    //
    const auto& scheduler = Scheduler::make();
    EXPECT_FALSE(&scheduler == nullptr);

    //
    EXPECT_EQ(&scheduler, &Scheduler::get());

    //
    Scheduler::destroy();
    EXPECT_EQ(&Scheduler::get(), nullptr);
}

TEST(SchedulerModule, Lifecycle) {
    //
    EXPECT_EQ(&Scheduler::get(), nullptr);

    //
    auto& scheduler = Scheduler::make();
    EXPECT_FALSE(&scheduler == nullptr);

    EXPECT_EQ(scheduler.getWorkerCount(), 0);

    //
    scheduler.setup(0);
    EXPECT_GE(scheduler.getWorkerCount(), 1);

    //
    // TODO: Make better solution to wait for responsiveness of workers
    _STD this_thread::sleep_for(_STD chrono::milliseconds(5000));

    //
    scheduler.tidy();
    EXPECT_EQ(scheduler.getWorkerCount(), 0);

    //
    Scheduler::destroy();
    EXPECT_EQ(&Scheduler::get(), nullptr);
}
