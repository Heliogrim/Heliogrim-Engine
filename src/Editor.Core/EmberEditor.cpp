#include "EmberEditor.hpp"

#include "EditorEngine.hpp"

using namespace ember::editor;
using namespace ember;

Session EmberEditor::getEditorSession() noexcept {
    auto* session {
        static_cast<ptr<EditorEngine>>(Ember::_engine.get())->getEditorSession()
    };

    managed<void> dummy {};
    return Session {
        managed<void> { _STD move(dummy), session }
    };
}

Session EmberEditor::getSession() {
    auto* session {
        static_cast<ptr<EditorEngine>>(Ember::_engine.get())->getPrimaryGameSession()
    };

    managed<void> dummy {};
    return Session {
        managed<void> { _STD move(dummy), session }
    };
}

Session EmberEditor::getSession(std::nothrow_t) noexcept {
    if (not Ember::_engine) {
        return Session {};
    }

    return EmberEditor::getSession();
}
