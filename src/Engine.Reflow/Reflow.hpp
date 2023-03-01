#pragma once

#include <Engine.Core/Module/SubModule.hpp>

namespace hg::engine {
    class Reflow :
        public core::SubModule {
    public:
        using this_type = Reflow;

    public:
        Reflow(const non_owning_rptr<Engine> engine_);

        ~Reflow() override;

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
    };
}
