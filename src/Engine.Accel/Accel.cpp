#include "Accel.hpp"

#include <Engine.Accel.Loader/Manager.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/CoreDependencies.hpp>
#include <Engine.Core/Module/DependencyKey.hpp>
#include <Engine.Core/Module/SubModuleDependency.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>

using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

Accel::Accel(const non_owning_rptr<Engine> engine_) :
    SubModule(engine_),
    _dependencies(
        {
            SubModuleDependency {
                GraphicsDepKey,
                SubModuleOrder::eSuccessor,
                true
            }
        }
    ) {}

Accel::~Accel() = default;

core::DependencyKey Accel::moduleKey() const noexcept {
    return AccelDepKey;
}

cref<CompactSet<core::SubModuleDependency>> Accel::dependencies() const noexcept {
    return _dependencies;
}

void Accel::setup() {

    _globalStorage = make_smr<accel::GlobalStorage>();

    /**
     * Register Hooks and Loader
     */
    auto& loader = _engine->getResources()->loader();
    accel::register_loader(loader, clone(_globalStorage));
    // accel::register_importer(...);
}

void Accel::start() {}

void Accel::stop() {}

void Accel::destroy() {

    /**
     * Unregister Hooks and Loader
     */
    auto& loader = _engine->getResources()->loader();
    accel::unregister_loader(loader, _globalStorage);

    /**/

    _globalStorage.reset();
}

cref<smr<accel::GlobalStorage>> Accel::getGlobalStorage() const noexcept {
    return _globalStorage;
}
