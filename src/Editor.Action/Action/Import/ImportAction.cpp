#include "ImportAction.hpp"

using namespace ember::editor;
using namespace ember;

ImportAction::ImportAction(cref<action_type_id> typeId_) :
    Action(typeId_) {}

ImportAction::~ImportAction() noexcept = default;
