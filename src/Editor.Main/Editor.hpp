#pragma once

#include <Engine.Core/Module/SubModule.hpp>

namespace ember::editor {
    class Editor :
        public engine::core::SubModule {
    public:
        using this_type = Editor;

    public:
        Editor(const non_owning_rptr<engine::Engine> engine_);

        ~Editor() override;

    private:
        CompactSet<engine::core::SubModuleDependency> _dependencies;

    public:
        [[nodiscard]] engine::core::DependencyKey moduleKey() const noexcept override;

        [[nodiscard]] cref<CompactSet<engine::core::SubModuleDependency>> dependencies() const noexcept override;

    public:
        void setup() override;

        void start() override;

        void stop() override;

        void destroy() override;
    };
}
