#include "AssetSeeder.hpp"

#include <Ember/Asset.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

using namespace ember;

engine::assets::AssetSeeder::AssetSeeder() = default;

engine::assets::AssetSeeder::~AssetSeeder() noexcept = default;

bool engine::assets::AssetSeeder::autoRegister(ptr<Asset> (* fncPtr_)()) {
    _constructors.push_back(fncPtr_);
    return true;
}

void engine::assets::AssetSeeder::seed(ptr<void> scheduler_) {

    Scheduler& scheduler { *static_cast<ptr<Scheduler>>(scheduler_) };

    /**
     * Iterate over registered constructors
     */
    const auto count { _constructors.size() };
    const auto perBatch { count / scheduler.getWorkerCount() };
    const auto lastBatch { count - (perBatch * (scheduler.getWorkerCount() - 1ui64)) };

    Vector<ember::concurrent::future<void>> batches {};
    batches.reserve(scheduler.getWorkerCount());

    auto iter { _constructors.begin() };
    for (auto batchIdx { scheduler.getWorkerCount() }; batchIdx >= 1ui64; --batchIdx) {

        const auto last { iter + (batchIdx == 1ui64 ? lastBatch : perBatch) };

        auto promise = ember::concurrent::promise<void>([first = iter, last]() {
            auto it { first };
            while (it != last) {
                delete (*(it++))();
            }
        });

        batches.push_back(promise.get());

        scheduler.exec(scheduler::task::make_task(promise));

        iter += (batchIdx == 1ui64 ? lastBatch : perBatch);
    }

    #ifdef _DEBUG
    assert(iter == _constructors.end());
    #endif

    /**
     * Wait until every batch settled
     */
    for (const auto& entry : batches) {
        entry.get();
    }
}
