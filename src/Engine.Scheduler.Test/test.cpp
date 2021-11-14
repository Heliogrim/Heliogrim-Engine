#include "pch.h"

#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Queue/ProcessingQueue.hpp>
#include <Engine.Scheduler/Queue/SharedBufferPool.hpp>

using namespace ember::engine::scheduler;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace SchedulerModule {

    TEST(SharedBufferPool, Default) {
        //
        auto pool = SharedBufferPool();

        //
        EXPECT_EQ(pool.capacity(), 0);
        EXPECT_EQ(pool.size().load(_STD memory_order_relaxed), 0);
    }

    TEST(SharedBufferPool, Reserve) {
        //
        auto pool = SharedBufferPool();

        //
        EXPECT_EQ(pool.capacity(), 0);
        EXPECT_EQ(pool.size().load(_STD memory_order_relaxed), 0);

        //
        pool.reserve(32ui32);

        //
        EXPECT_EQ(pool.capacity(), 32);
        EXPECT_EQ(pool.size().load(_STD memory_order_relaxed), 32);
    }

    TEST(ProcessingQueue, Default) {
        //
        auto pool = engine::scheduler::SharedBufferPool();
        auto queue = engine::scheduler::ProcessingQueue(&pool);

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 0);
    }

    TEST(ProcessingQueue, Write) {
        //
        auto pool = engine::scheduler::SharedBufferPool();
        auto queue = engine::scheduler::ProcessingQueue(&pool);

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 0);

        //
        auto task = engine::scheduler::task::make_task([]() {});
        queue.push(_STD move(task));

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 1);
    }

    TEST(ProcessingQueue, WriteMany) {
        //
        auto pool = engine::scheduler::SharedBufferPool();
        auto queue = engine::scheduler::ProcessingQueue(&pool);

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 0);

        //
        constexpr auto req = 16ui32 * 31ui32 + 1ui32;
        vector<task::__TaskDelegate> tasks {};

        tasks.reserve(req);
        for (auto ci = req; ci > 0; --ci) {
            tasks.push_back(engine::scheduler::task::make_task([]() {}));
        }

        //
        for (auto& entry : tasks) {
            queue.push(_STD move(entry));
        }

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 2);

        // TODO: Cleanup
    }

    TEST(ProcessingQueue, Cycle) {
        //
        auto pool = engine::scheduler::SharedBufferPool();
        auto queue = engine::scheduler::ProcessingQueue(&pool);

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 0);

        //
        auto task = engine::scheduler::task::make_task([]() {});
        queue.push(_STD move(task));

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 1);

        //
        task::__TaskDelegate dstTask = nullptr;
        queue.pop(task::TaskMask::eAll, dstTask);

        //
        EXPECT_NE(dstTask, nullptr);
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 1);
    }

    TEST(ProcessingQueue, CycleMany) {
        //
        auto pool = engine::scheduler::SharedBufferPool();
        auto queue = engine::scheduler::ProcessingQueue(&pool);

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 0);

        //
        constexpr auto req = 16ui32 * 31ui32 * 3ui32 + 1ui32;
        vector<task::__TaskDelegate> tasks {};

        tasks.reserve(req);
        for (auto ci = req; ci > 0; --ci) {
            tasks.push_back(engine::scheduler::task::make_task([]() {}));
        }

        //
        for (auto& entry : tasks) {
            queue.push(_STD move(entry));
        }

        //
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 4);

        //
        task::__TaskDelegate dstTask = nullptr;
        queue.pop(task::TaskMask::eAll, dstTask);

        //
        EXPECT_NE(dstTask, nullptr);
        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 4);

        //
        for (auto i = 0ui32; i < 31ui32; ++i) {
            dstTask = nullptr;
            queue.pop(task::TaskMask::eAll, dstTask);
            EXPECT_NE(dstTask, nullptr);
        }

        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 4);

        //
        for (auto i = 0ui32; i < 31ui32; ++i) {
            dstTask = nullptr;
            queue.pop(task::TaskMask::eAll, dstTask);
            EXPECT_NE(dstTask, nullptr);
        }

        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 4);

        //
        for (auto i = 0ui32; i < 31ui32; ++i) {
            dstTask = nullptr;
            queue.pop(task::TaskMask::eAll, dstTask);
            EXPECT_NE(dstTask, nullptr);
        }

        EXPECT_EQ(queue.pageCount().load(_STD memory_order_relaxed), 4);

        // TODO: Cleanup
    }
}
