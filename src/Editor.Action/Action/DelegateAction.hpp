#pragma once

#include <functional>

#include "SimpleAction.hpp"

namespace hg::editor {
    class DelegateAction :
        public SimpleAction {
    public:
        using this_type = DelegateAction;
        using delegate_type = _STD function<void()>;

    public:
        inline constexpr static action_type_id typeId { "DelegateAction"_typeId };

    protected:
        DelegateAction(cref<action_type_id> typeId_);

    public:
        DelegateAction();

        DelegateAction(mref<delegate_type> fwd_, mref<delegate_type> bwd_);

        ~DelegateAction() override;

    private:
        delegate_type _fwd;
        delegate_type _bwd;

    public:
        void store(mref<delegate_type> fwd_, mref<delegate_type> bwd_);

    public:
        [[nodiscard]] bool isReversible() const noexcept override;

    public:
        void apply() override;

        void reverse() override;

    public:
        [[nodiscard]] bool failed() const noexcept override;
    };
}
