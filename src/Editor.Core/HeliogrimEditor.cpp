#include "HeliogrimEditor.hpp"

#include "EditorEngine.hpp"

using namespace hg::editor;
using namespace hg;

Session HeliogrimEditor::getEditorSession() noexcept {
    auto* session {
        static_cast<ptr<EditorEngine>>(Heliogrim::_engine.get())->getEditorSession()
    };

    managed<void> dummy {};
    return Session {
        managed<void> { std::move(dummy), session }
    };
}

Session HeliogrimEditor::getSession() {
    auto* session {
        static_cast<ptr<EditorEngine>>(Heliogrim::_engine.get())->getPrimaryGameSession()
    };

    managed<void> dummy {};
    return Session {
        managed<void> { std::move(dummy), session }
    };
}

Session HeliogrimEditor::getSession(std::nothrow_t) noexcept {
    if (not Heliogrim::_engine) {
        return Session {};
    }

    return HeliogrimEditor::getSession();
}
