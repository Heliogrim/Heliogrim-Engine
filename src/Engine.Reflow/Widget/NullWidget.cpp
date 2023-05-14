#include "NullWidget.hpp"

#include "../Children.hpp"

#include <Engine.Common/Make.hpp>
#include <format>

using namespace hg::engine::reflow;
using namespace hg;

NullWidget::NullWidget() :
    Widget() {}

NullWidget::~NullWidget() = default;

sptr<NullWidget> NullWidget::instance() {
    static sptr<NullWidget> instance = sptr<NullWidget>(new NullWidget());
    return instance;
}

string NullWidget::getTag() const noexcept {
    return _STD format(R"(NullWidget <{:#x}>)", reinterpret_cast<u64>(this));
}

const ptr<const Children> NullWidget::children() const {
    return get_null_children();
}

void NullWidget::render(const ptr<ReflowCommandBuffer> cmd_) {}

math::vec2 NullWidget::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
    return math::vec2 {};
}

math::vec2 NullWidget::computeDesiredSize(cref<ReflowPassState> passState_) const {
    return math::vec2 {};
}

void NullWidget::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {}

bool NullWidget::willChangeLayout(cref<math::vec2> space_) const noexcept {
    return false;
}
