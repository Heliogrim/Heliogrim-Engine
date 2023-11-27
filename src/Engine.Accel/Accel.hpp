#pragma once

#include <Engine.Accel.Storage/GlobalStorage.hpp>
#include <Engine.Core/Module/SubModule.hpp>
#include <Engine.Core/Module/DependencyKey.hpp>

namespace hg::engine {
    inline constexpr static core::DependencyKey AccelDepKey { "::hg::engine::accel::Accel"sv };
}

namespace hg::engine {
    class Accel :
        public core::SubModule {
    public:
        using this_type = Accel;

    public:
        Accel(const non_owning_rptr<Engine> engine_);

        ~Accel() override;

    private:
        CompactSet<core::SubModuleDependency> _dependencies;

    public:
        [[nodiscard]] core::DependencyKey moduleKey() const noexcept override;

        [[nodiscard]] cref<CompactSet<core::SubModuleDependency>> dependencies() const noexcept override;

    public:
        void setup() override;

        void start() override;

        void stop() override;

        void destroy() override;

    private:
        smr<accel::GlobalStorage> _globalStorage;

    public:
        [[nodiscard]] cref<smr<accel::GlobalStorage>> getGlobalStorage() const noexcept;
    };
}
