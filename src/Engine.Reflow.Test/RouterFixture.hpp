#pragma once

#include "TreeFixture.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;
#endif

namespace ReflowModule {
	using namespace hg;

	/**
	 * Widget which records every routed event into a shared log and answers with a configurable reply.
	 */
	template <class Base_ = engine::reflow::uikit::Paint>
	class ProbeWidget final :
		public Base_ {
	public:
		using MouseEvent = engine::reflow::MouseEvent;
		using MouseMoveEvent = engine::reflow::MouseMoveEvent;
		using KeyboardEvent = engine::reflow::KeyboardEvent;
		using WheelEvent = engine::reflow::WheelEvent;
		using DragDropEvent = engine::reflow::DragDropEvent;
		using EventReply = engine::reflow::EventReply;

	public:
		ProbeWidget(string name_, const ptr<Vector<string>> log_) :
			Base_(),
			name(std::move(name_)),
			log(log_) {}

	public:
		string name;
		ptr<Vector<string>> log;

		EventReply previewDown = EventReply::unhandled();
		EventReply down = EventReply::unhandled();
		EventReply up = EventReply::unhandled();
		EventReply move = EventReply::unhandled();
		EventReply wheel = EventReply::unhandled();
		EventReply drop = EventReply::unhandled();
		EventReply previewKey = EventReply::unhandled();
		EventReply keyDown = EventReply::unhandled();

		bool focusable = false;
		std::function<void()> blurHook {};

	private:
		EventReply record(ref<const string> kind_, ref<const EventReply> reply_) {
			log->emplace_back(kind_ + ":" + name);
			return reply_;
		}

	public:
		EventReply invokeOnPreviewMouseButtonDown(ref<const MouseEvent>) override {
			return record("preview-down", previewDown);
		}

		EventReply invokeOnMouseButtonDown(ref<const MouseEvent>) override {
			return record("down", down);
		}

		EventReply invokeOnMouseButtonUp(ref<const MouseEvent>) override {
			return record("up", up);
		}

		EventReply invokeOnMouseMove(ref<const MouseMoveEvent>) override {
			return record("move", move);
		}

		EventReply invokeOnMouseEnter(ref<const MouseMoveEvent>) override {
			return record("enter", EventReply::unhandled());
		}

		EventReply invokeOnMouseLeave(ref<const MouseMoveEvent>) override {
			return record("leave", EventReply::unhandled());
		}

		EventReply invokeOnWheel(ref<const WheelEvent>) override {
			return record("wheel", wheel);
		}

		EventReply invokeOnDrop(ref<const DragDropEvent>) override {
			return record("drop", drop);
		}

		EventReply invokeOnPreviewKeyDown(ref<const KeyboardEvent>) override {
			return record("preview-key", previewKey);
		}

		EventReply invokeOnKeyDown(ref<const KeyboardEvent>) override {
			return record("key-down", keyDown);
		}

		[[nodiscard]] bool isFocusable() const noexcept override {
			return focusable;
		}

		EventReply invokeOnBlur(ref<const engine::reflow::FocusEvent> event_) override {
			if (const auto hook = std::exchange(blurHook, {})) {
				hook();
			}
			return Base_::invokeOnBlur(event_);
		}
	};

	/**
	 * Window 320x240 with content a(200x200) > b(100x100) > c(20x20), registered on an owned router.
	 */
	class RouterFixture :
		public TreeFixture {
	public:
		using Container = ProbeWidget<engine::reflow::uikit::VerticalLayout>;
		using Leaf = ProbeWidget<>;

	public:
		Vector<string> log;
		engine::reflow::EventRouter router;
		sptr<engine::reflow::Window> window;
		sptr<Container> a;
		sptr<Container> b;
		sptr<Leaf> c;

	public:
		template <class Type_>
		static void size(ref<const sptr<Type_>> widget_, const float extent_) {
			const auto unit = engine::reflow::ReflowUnit { .type = engine::reflow::ReflowUnitType::eAbsolute, .value = extent_ };
			auto& attributes = [&]() -> decltype(auto) {
				if constexpr (requires { widget_->getLayoutAttributes().attributeSets; }) {
					return (std::get<0>(widget_->getLayoutAttributes().attributeSets));

				} else {
					return (widget_->getLayoutAttributes());
				}
			}();
			attributes.template update<engine::reflow::attr::BoxLayout::minWidth>(unit);
			attributes.template update<engine::reflow::attr::BoxLayout::maxWidth>(unit);
			attributes.template update<engine::reflow::attr::BoxLayout::minHeight>(unit);
			attributes.template update<engine::reflow::attr::BoxLayout::maxHeight>(unit);
		}

		void SetUp() override {
			a = make_sptr<Container>("a", &log);
			b = make_sptr<Container>("b", &log);
			c = make_sptr<Leaf>("c", &log);
			size(a, 200.F);
			size(b, 100.F);
			size(c, 20.F);
			b->addChild(c);
			a->addChild(b);

			window = make_sptr<engine::reflow::Window>();
			window->setClientSize(math::vec2 { 320.F, 240.F });
			window->setContent(a);
			router.registerWindow(window);
			tick(window);
		}

		void TearDown() override {
			router.unregisterWindow(window.get());
			TreeFixture::TearDown();
		}

		[[nodiscard]] Vector<string> filtered(ref<const string> prefix_) const {
			Vector<string> result {};
			for (const auto& entry : log) {
				if (entry.starts_with(prefix_)) {
					result.emplace_back(entry);
				}
			}
			return result;
		}

		engine::reflow::EventReply mouse(const s32 x_, const s32 y_, const bool down_, const u32 button_ = 0u) {
			return router.route(window, engine::input::event::MouseButtonEvent { math::ivec2 { x_, y_ }, button_, down_, 0u });
		}

		engine::reflow::EventReply key(const char key_, const u16 modifier_ = 0u) {
			return router.route(window, engine::input::event::KeyboardEvent { key_, true, modifier_ });
		}

		void mount(cref<sptr<engine::reflow::Widget>> content_) {
			window->setContent(content_);
			tick(window);
		}

		engine::reflow::EventReply move(const float x_, const float y_) {
			return router.route(window, engine::input::event::MouseMoveEvent { math::fvec2 { x_, y_ }, math::fvec2 { 0.F }, 0u, 0u });
		}
	};
}
