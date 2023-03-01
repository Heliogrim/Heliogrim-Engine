#pragma once

#include "../Action.hpp"

namespace hg::editor {
    class ImportAction :
        public Action {
    public:
        using this_type = ImportAction;

    protected:
        ImportAction(cref<action_type_id> typeId_);

    public:
        ~ImportAction() noexcept override;
    };
}
