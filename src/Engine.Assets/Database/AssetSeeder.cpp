#include "AssetSeeder.hpp"

#include <Ember/Asset.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

using namespace ember;

engine::assets::AssetSeeder::AssetSeeder() = default;

engine::assets::AssetSeeder::~AssetSeeder() noexcept = default;

bool engine::assets::AssetSeeder::autoRegister(ptr<Asset> (* fncPtr_)()) {
    _constructors.push_back(fncPtr_);
    return true;
}

void engine::assets::AssetSeeder::seed(ptr<void> scheduler_) {

    scheduler::Scheduler& scheduler { *static_cast<ptr<scheduler::Scheduler>>(scheduler_) };

    /**
     * Iterate over registered constructors
     */
    for (auto entry : _constructors) {
        scheduler.exec(scheduler::task::make_task([entry = entry]() {
            delete entry();
        }));
    }
}
