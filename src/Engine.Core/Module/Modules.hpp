#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Deque.hpp>
#include <Engine.Common/Collection/Set.hpp>

#include "DependencyKey.hpp"
#include "DependencyValidationResult.hpp"

namespace ember::engine::core {
    class SubModule;
    struct SubModuleDependency;
}

namespace ember::engine::core {
    class Modules {
    public:
        using this_type = Modules;

    public:
        Modules();

        Modules(cref<this_type>) = delete;

        Modules(mref<this_type>) noexcept = delete;

        ~Modules();

    private:
        Deque<uptr<SubModule>> _subModules;
        CompactSet<DependencyKey> _coreModules;

    private:
        [[nodiscard]] bool validateDependency(cref<SubModuleDependency> dependency_) const noexcept;

    public:
        [[nodiscard]] cref<Deque<uptr<SubModule>>> getSubModules() const noexcept;

    public:
        DependencyValidationResult addSubModule(mref<uptr<SubModule>> subModule_);

        [[nodiscard]] const non_owning_rptr<SubModule> getSubModule(cref<DependencyKey> moduleKey_) const noexcept;

        bool removeSubModule(const non_owning_rptr<SubModule> subModule_);

    public:
        [[nodiscard]] DependencyValidationResult validate() const noexcept;
    };
}
