#include "pch.h"

/**/

#include <Engine.Common/__macro.hpp>

import Editor.UI;
import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg;

namespace ReflowModule {
	static void size(cref<sptr<engine::reflow::uikit::Paint>> widget_, const float width_, const float height_) {
		auto& attributes = widget_->getLayoutAttributes();
		attributes.update<engine::reflow::attr::BoxLayout::minWidth>({ engine::reflow::ReflowUnitType::eAbsolute, width_ });
		attributes.update<engine::reflow::attr::BoxLayout::maxWidth>({ engine::reflow::ReflowUnitType::eAbsolute, width_ });
		attributes.update<engine::reflow::attr::BoxLayout::minHeight>({ engine::reflow::ReflowUnitType::eAbsolute, height_ });
		attributes.update<engine::reflow::attr::BoxLayout::maxHeight>({ engine::reflow::ReflowUnitType::eAbsolute, height_ });
	}

	/**
	 * Stand-in for the ported editor InputSelect: focusable, focuses itself on down and opens its menu through the router.
	 */
	class PopupSelect final :
		public engine::reflow::uikit::Paint {
	public:
		Opt<engine::reflow::PopupHandle> menu = None;

	public:
		[[nodiscard]] bool isFocusable() const noexcept override {
			return true;
		}

		engine::reflow::EventReply invokeOnMouseButtonDown(ref<const engine::reflow::MouseEvent> event_) override {
			if (event_._button != 1u) {
				return Paint::invokeOnMouseButtonDown(event_);
			}

			const auto manager = engine::reflow::WindowManager::get();
			if (menu.has_value()) {
				manager->closePopup(*menu);
				return engine::reflow::EventResponse::eConsumed;
			}

			const auto window = std::static_pointer_cast<engine::reflow::Window>(root());
			manager->focus(window, shared_from_this());

			auto content = make_sptr<engine::reflow::uikit::Paint>();
			size(content, 200.F, 160.F);

			const auto& layout = getLayoutState();
			menu = manager->openPopup(
				window,
				::hg::move(content),
				engine::reflow::PopupAnchor {
					.offset = layout.layoutOffset, .size = layout.layoutSize,
					.side = engine::reflow::PopupSide::eBelow, .align = engine::reflow::PopupAlign::eStart
				},
				weak_from_this(),
				None,
				[this]() { menu = None; }
			);
			return engine::reflow::EventResponse::eConsumed;
		}
	};

	/**
	 * The editor's property service needs a live engine (assets, components), so the panel is built from the same
	 *  widgets it hands out: three uikit inputs and an InputSelect stand-in (the real select crashes in openMenu without an engine) inside a FocusScope(ePanel).
	 */
	class PanelFocusFixture :
		public testing::Test {
	public:
		struct TestWindowManager :
			public engine::reflow::WindowManager {
			static void install() {
				_instance = sptr<engine::reflow::WindowManager>(new TestWindowManager());
			}
		};

	public:
		engine::reflow::LayoutContext ctx {
			.localOffset = math::vec2 { 0.F },
			.localSize = math::vec2 { 320.F, 240.F },
			.localScale = 1.F
		};
		engine::reflow::ReflowState state;
		sptr<engine::reflow::Window> window;

		sptr<engine::reflow::uikit::InputText> text;
		sptr<engine::reflow::uikit::InputFloat> real;
		sptr<engine::reflow::uikit::InputIntegral> integral;
		sptr<PopupSelect> select;

	public:
		[[nodiscard]] static ref<engine::reflow::WindowManager> router() {
			return *engine::reflow::WindowManager::get();
		}

		void tick() {
			engine::reflow::ReflowEngine::tick(state, window, clone(ctx));
		}

		void SetUp() override {
			TestWindowManager::install();

			text = engine::reflow::uikit::makeInputText();
			real = engine::reflow::uikit::makeInputFloat();
			integral = engine::reflow::uikit::makeInputIntegral();
			select = make_sptr<PopupSelect>();
			size(select, 120.F, 20.F);

			auto column = make_sptr<engine::reflow::uikit::VerticalLayout>();
			column->addChild(text);
			column->addChild(select);
			column->addChild(real);
			column->addChild(integral);

			window = make_sptr<engine::reflow::Window>();
			window->setClientSize(math::vec2 { 320.F, 240.F });
			auto scope = make_sptr<engine::reflow::FocusScope>(engine::reflow::FocusScopeKind::ePanel);
			window->setContent(scope);
			scope->setContent(column);
			router().registerWindow(window);
			tick();
		}

		void TearDown() override {
			router().unregisterWindow(window.get());
			state.reset();
			engine::reflow::WindowManager::destroy();
		}

		[[nodiscard]] static math::ivec2 center(cref<sptr<engine::reflow::Widget>> widget_) {
			const auto& layout = widget_->getLayoutState();
			return math::ivec2 {
				static_cast<s32>(layout.layoutOffset.x + layout.layoutSize.x * 0.5F),
				static_cast<s32>(layout.layoutOffset.y + layout.layoutSize.y * 0.5F)
			};
		}

		engine::reflow::EventReply mouse(cref<math::ivec2> at_, const bool down_) {
			return router().route(window, engine::input::event::MouseButtonEvent { at_, 1u, down_, 0u });
		}

		engine::reflow::EventReply key(const char key_, const u16 modifier_ = 0u) {
			return router().route(window, engine::input::event::KeyboardEvent { key_, true, modifier_ });
		}
	};

	TEST_F(PanelFocusFixture, TabCyclesPropertyInputs) {
		router().focus(window, text);

		key('\t');
		EXPECT_EQ(window->getFocusTarget(), select);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), real);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), integral);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), text);

		key('\t', 0x0001u);
		EXPECT_EQ(window->getFocusTarget(), integral);
	}

	TEST_F(PanelFocusFixture, SelectPopupBlocksBelowAndRestores) {
		router().focus(window, text);

		const auto at = center(select);
		ASSERT_GT(select->getLayoutState().layoutSize.y, 0.F);
		mouse(at, true);
		mouse(at, false);
		tick();
		ASSERT_TRUE(router().hasOpenPopup(window.get()));

		// The menu covers the fields below; a down there must not land on them.
		mouse(center(real), true);
		mouse(center(real), false);
		EXPECT_TRUE(router().hasOpenPopup(window.get()));
		EXPECT_FALSE(real->state().isFocus());
		EXPECT_FALSE(integral->state().isFocus());

		key(static_cast<char>(0x1B));
		tick();
		EXPECT_FALSE(router().hasOpenPopup(window.get()));
		EXPECT_EQ(window->getFocusTarget(), select);
	}
}
