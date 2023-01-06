#pragma once

#include <Engine.Scheduler/CompScheduler.hpp>

namespace ember::engine {
    class TestCompScheduler final :
        public CompScheduler {
    public:
        TestCompScheduler() :
            CompScheduler() {}

        ~TestCompScheduler() override = default;

    public:
        [[nodiscard]] ptr<scheduler::Schedule> testGetSchedule() const noexcept {
            return _schedule.get();
        }
    };
}
