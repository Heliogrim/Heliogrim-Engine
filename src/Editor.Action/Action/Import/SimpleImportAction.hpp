#pragma once
#include "ImportAction.hpp"

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Assets/Types/Asset.hpp>

namespace ember::editor {
    class SimpleImportAction :
        public ImportAction {
    public:
        using this_type = SimpleImportAction;

        inline constexpr static action_type_id typeId { "SimpleImportAction"_typeId };

    public:
        SimpleImportAction();

        SimpleImportAction(cref<Url> source_, cref<Url> target_);

        ~SimpleImportAction();

    private:
        Url _source;
        Url _target;

        // TODO: ptr<void> _options;

    public:
        [[nodiscard]] Url getSourceUrl() const noexcept;

        [[nodiscard]] Url getTargetUrl() const noexcept;

    private:
        CompactSet<ptr<::ember::engine::assets::Asset>> _assets;

    public:
        [[nodiscard]] cref<decltype(_assets)> importedAssets() const noexcept;

    private:
        _STD atomic_flag _running;
        _STD atomic_flag _finished;

    protected:
        void setRunning();

        void setFinished();

    public:
        [[nodiscard]] bool isReversible() const noexcept override;

        [[nodiscard]] bool isPending() const noexcept override;

        [[nodiscard]] bool isRunning() const noexcept override;

        [[nodiscard]] bool isFinished() const noexcept override;

    public:
        void apply() override;

        void reverse() override;

    public:
        [[nodiscard]] operator ptr<await_signal_sub_type>() const noexcept;

    private:
        bool _failed;

    public:
        [[nodiscard]] bool failed() const noexcept override;
    };
}
