#include "pch.h"

/**/

#include <Editor.UI/Widget/Breadcrumb.hpp>
#include <Editor.UI/Widget/Collapse.hpp>
#include <Editor.UI/Widget/MenuItem.hpp>
#include <Editor.UI/Widget/MenuSpacer.hpp>
#include <Engine.Common/Memory/SharedPointer.hpp>
#include <Engine.Reflow/ReflowEngine.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow/Widget/Viewport.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>

using namespace hg;

namespace ReflowModule {
	class LayoutingBoxFixture :
		public testing::Test {
	public:
		constexpr static auto default_test_offset = math::vec2 { 0.f };
		constexpr static auto default_test_dimension = math::vec2 { 320.f, 240.f };
		constexpr static auto default_test_scale = 1.f;

	protected:
		engine::reflow::LayoutContext _reflowContext;
		engine::reflow::ReflowState _reflowState;
		SharedPtr<engine::reflow::uikit::BoxLayout> _box;

	public:
		void attach(auto&& widget_) {
			_box->setChild(::hg::forward<decltype(widget_)>(widget_));
		}

		void computeLayout() {
			engine::reflow::ReflowEngine::tick(_reflowState, _box, clone(_reflowContext));
		}

	public:
		void SetUp() override {
			_box = make_sptr<engine::reflow::uikit::BoxLayout>();
			_reflowState = engine::reflow::ReflowState {};
			_reflowContext = engine::reflow::LayoutContext { default_test_offset, default_test_dimension, default_test_scale };

			_box->getLayoutAttributes().update<engine::reflow::attr::BoxLayout::placement>(engine::reflow::ReflowPlacement::eTopLeft);

			auto& layout = _box->getLayoutState();
			layout.layoutOffset = default_test_offset;
			layout.layoutSize = default_test_dimension;
			layout.computeSize = default_test_dimension;
			layout.prefetchMaxSize = default_test_dimension;
			layout.prefetchMinSize = default_test_dimension;
			layout.prefetchSize = default_test_dimension;
			layout.referenceSize = default_test_dimension;
		}

		void TearDown() override {
			ASSERT_EQ(_box.use_count(), 1);

			_reflowState.reset();
			_box.reset();
		}
	};

	template <typename WidgetType_>
	constexpr static auto generic_test_empty(ref<LayoutingBoxFixture> self_, const math::vec2 expectedSize_ = math::vec2 { 0.f }) {

		auto subject = make_sptr<WidgetType_>();
		self_.attach(subject);

		self_.computeLayout();

		const auto& layout = subject->getLayoutState();
		EXPECT_TRUE(layout.layoutOffset.allZero());
		EXPECT_EQ(layout.layoutSize, expectedSize_);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitBreadcrumb) {
		GTEST_SKIP() << "Will currently segfault due to the internal usage of uikit::Text."sv;
		generic_test_empty<editor::ui::Breadcrumb>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitCollapse) {
		generic_test_empty<editor::ui::Collapse>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitMenuItem) {
		generic_test_empty<editor::ui::MenuItem>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitMenuSpacer) {
		generic_test_empty<editor::ui::MenuSpacer>(*this);
	}
}
