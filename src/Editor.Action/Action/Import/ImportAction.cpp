#include "ImportAction.hpp"

using namespace hg::editor;
using namespace hg;

ImportAction::ImportAction(cref<action_type_id> typeId_) :
    Action(typeId_) {}

ImportAction::~ImportAction() noexcept = default;
