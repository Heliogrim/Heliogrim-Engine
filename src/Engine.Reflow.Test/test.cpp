#include "pch.h"

/**/

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

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
	constexpr static auto generic_test_empty_input(
		ref<LayoutingBoxFixture> self_,
		auto&& factoryFn_,
		const math::vec2 expectedSize_ = math::vec2 { 0.f }
	) {

		auto subject = factoryFn_();
		self_.attach(subject);

		self_.computeLayout();

		const auto& layout = subject->getLayoutState();
		EXPECT_TRUE(layout.layoutOffset.allZero());
		EXPECT_EQ(layout.layoutSize, expectedSize_);
	}

	template <typename WidgetType_>
	constexpr static auto generic_test_empty(ref<LayoutingBoxFixture> self_, const math::vec2 expectedSize_ = math::vec2 { 0.f }) {
		generic_test_empty_input<WidgetType_>(self_, []() { return make_sptr<WidgetType_>(); }, expectedSize_);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitBox) {
		generic_test_empty<engine::reflow::uikit::BoxLayout>(*this);
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
		// Note: Without a font the text contributes no measure, only its default padding (4 per side) remains.
		generic_test_empty<engine::reflow::uikit::Text>(*this, math::vec2 { 8.f });
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

	TEST_F(LayoutingBoxFixture, EmptyUikitInputFloat) {
		generic_test_empty_input<engine::reflow::uikit::InputFloat>(*this, []() { return engine::reflow::uikit::makeInputFloat(); }, math::vec2 { 8.f });
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitInputIntegral) {
		generic_test_empty_input<engine::reflow::uikit::InputIntegral>(*this, []() { return engine::reflow::uikit::makeInputIntegral(); }, math::vec2 { 8.f });
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitInputText) {
		generic_test_empty_input<engine::reflow::uikit::InputText>(*this, []() { return engine::reflow::uikit::makeInputText(); }, math::vec2 { 8.f });
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitInputToggle) {
		generic_test_empty<engine::reflow::uikit::InputToggle>(*this);
	}

	// Fenced: InputVec<T>::prepare() no longer compiles (make_sptr<InputFloat>() has no default ctor, InputFloat::_callback is gone).
	#if FALSE

	TEST_F(LayoutingBoxFixture, EmptyUikitInputVec3) {
		generic_test_empty<engine::reflow::uikit::InputVec3>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitInputUiVec2) {
		generic_test_empty<engine::reflow::uikit::InputUiVec2>(*this);
	}

	TEST_F(LayoutingBoxFixture, EmptyUikitInputUiVec3) {
		generic_test_empty<engine::reflow::uikit::InputUiVec3>(*this);
	}

	#endif

	/**/

	static void fixSize(ref<engine::reflow::uikit::BoxLayout> box_, const math::vec2 size_) {
		using namespace engine::reflow;
		auto& attr = box_.getLayoutAttributes();
		attr.update<attr::BoxLayout::minWidth>(ReflowUnit { ReflowUnitType::eAbsolute, size_.x });
		attr.update<attr::BoxLayout::maxWidth>(ReflowUnit { ReflowUnitType::eAbsolute, size_.x });
		attr.update<attr::BoxLayout::minHeight>(ReflowUnit { ReflowUnitType::eAbsolute, size_.y });
		attr.update<attr::BoxLayout::maxHeight>(ReflowUnit { ReflowUnitType::eAbsolute, size_.y });
	}

	TEST_F(LayoutingBoxFixture, FixedSizeBox) {
		auto subject = make_sptr<engine::reflow::uikit::BoxLayout>();
		fixSize(*subject, math::vec2 { 64.f, 32.f });
		attach(subject);

		computeLayout();

		const auto& layout = subject->getLayoutState();
		EXPECT_TRUE(layout.layoutOffset.allZero());
		EXPECT_EQ(layout.layoutSize, (math::vec2 { 64.f, 32.f }));
	}

	TEST_F(LayoutingBoxFixture, AutoBoxWrapsFixedChild) {
		auto child = make_sptr<engine::reflow::uikit::BoxLayout>();
		fixSize(*child, math::vec2 { 40.f, 20.f });

		auto subject = make_sptr<engine::reflow::uikit::BoxLayout>();
		subject->setChild(child);
		attach(subject);

		computeLayout();

		EXPECT_EQ(child->getLayoutState().layoutSize, (math::vec2 { 40.f, 20.f }));
		EXPECT_EQ(subject->getLayoutState().layoutSize, (math::vec2 { 40.f, 20.f }));
	}

	TEST_F(LayoutingBoxFixture, FixedSizeIsStableAcrossTicks) {
		auto subject = make_sptr<engine::reflow::uikit::BoxLayout>();
		fixSize(*subject, math::vec2 { 64.f, 32.f });
		attach(subject);

		computeLayout();
		const auto first = subject->getLayoutState().layoutSize;
		computeLayout();

		EXPECT_EQ(subject->getLayoutState().layoutSize, first);
	}
}
