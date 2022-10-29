#include "Form.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

Form::Form() :
    ReflowContainer() {

    _reflowType = ReflowType::eFlexRow;
    _reflowSpacing = ReflowSpacing::eStart;
    _reflowOverflow = ReflowOverflow::eNone;
    _reflowWrapping = ReflowWrapping::eNone;

    _extent.x = -1.F;
    _extent.y = -1.F;
    _minExtent.x = -1.F;
    _minExtent.y = -1.F;
    _maxExtent.x = -1.F;
    _maxExtent.y = -1.F;

    _reflowShrink = 1.F;
    _reflowGrow = 1.F;
}

Form::~Form() = default;

void Form::submitForm() {
    const auto event { event::FormSubmitEvent {} };
    _emitter.emit(event);
}

void Form::clearForm() {}

u64 Form::onSubmit(mref<std::function<void(cref<event::FormSubmitEvent> event_)>> listener_) {
    return _emitter.on<event::FormSubmitEvent>(_STD move(listener_));
}
