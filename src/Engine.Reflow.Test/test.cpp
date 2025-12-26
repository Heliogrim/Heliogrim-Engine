#include "pch.h"

/**/

#include <Engine.Common/Memory/SharedPointer.hpp>
#include <Engine.Reflow/ReflowEngine.hpp>
#include <Engine.Reflow.Uikit/Atom/Image.hpp>
#include <Engine.Reflow.Uikit/Atom/Paint.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Atom/ThemeProvider.hpp>
#include <Engine.Reflow/Widget/Viewport.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/UniformGridLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>
#include <Engine.Reflow/Widget/Layer/Anchor.hpp>
#include <Engine.Reflow/Widget/Menu/Menu.hpp>
#include <Engine.Reflow.Uikit/Exp/Button.hpp>
#include <Engine.Reflow.Uikit/Exp/Card.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/VScrollBox.hpp>

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

	TEST_F(LayoutingBoxFixture, EmptyNullWidget) {

		auto subject = engine::reflow::NullWidget::instance();
		attach(subject);

		computeLayout();

		const auto& layout = subject->getLayoutState();
		EXPECT_TRUE(layout.layoutOffset.allZero());
		EXPECT_TRUE(layout.layoutSize.allZero());
	}

	template <typename WidgetType_>
	constexpr static auto generic_test_empty(ref<LayoutingBoxFixture> self_, const math::vec2 expectedSize_ = math::vec2 { 0.f }) {

		auto subject = make_sptr<WidgetType_>();
		self_.attach(subject);

		self_.computeLayout();

		const auto& layout = subject->getLayoutState();
		EXPECT_TRUE(layout.layoutOffset.allZero());
		EXPECT_EQ(layout.layoutSize, expectedSize_);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitBox) {
		generic_test_empty<engine::reflow::uikit::BoxLayout>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitViewport) {
		GTEST_SKIP_("Will break due to deep integration with render system.");
		generic_test_empty<engine::reflow::Viewport>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitAnchor) {
		generic_test_empty<engine::reflow::Anchor>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitMenu) {
		generic_test_empty<engine::reflow::Menu>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitHorizontal) {
		generic_test_empty<engine::reflow::uikit::HorizontalLayout>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitVertical) {
		generic_test_empty<engine::reflow::uikit::VerticalLayout>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitUniformGrid) {
		generic_test_empty<engine::reflow::uikit::UniformGridLayout>(*this, math::vec2 { 8.f });
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitImage) {
		generic_test_empty<engine::reflow::uikit::Image>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitPaint) {
		generic_test_empty<engine::reflow::uikit::Paint>(*this, default_test_dimension);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitText) {
		GTEST_SKIP_("Will break due to missing font handling.");
		generic_test_empty<engine::reflow::uikit::Text>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitTheme) {
		generic_test_empty<engine::reflow::uikit::ThemeProvider>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitStack) {
		generic_test_empty<engine::reflow::uikit::Stack>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitVerticalScroll) {
		generic_test_empty<engine::reflow::uikit::VScrollBox>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitButton) {
		generic_test_empty<engine::reflow::uikit::Button>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitCard) {
		generic_test_empty<engine::reflow::uikit::Card>(*this);
	}
}
