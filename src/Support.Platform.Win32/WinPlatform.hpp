#pragma once

#include <Engine.Async/Affine/AffineThread.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Platform/Platform.hpp>

union SDL_Event;

namespace hg::engine {
	class WinPlatform final :
		public Platform {
	public:
		using this_type = WinPlatform;

	public:
		WinPlatform();

		~WinPlatform() override;

	private:
		void setupThread();

	public:
		void tidy() override;

		void setup() override;

	private:
		// Mutable: platformThread() is const but hands out the thread for posting.
		mutable async::AffineThread _platformThread;

		Vector<ptr<platform::NativeWindow>> _windows;

	public:
		[[nodiscard]] ref<async::AffineThread> platformThread() const noexcept override;

		[[nodiscard]] hg::Future<uptr<platform::NativeWindow>> makeNativeWindow(
			const string_view title_,
			cref<math::Extent2i32> extent_
		) override;

		[[nodiscard]] hg::Future<bool> destroyNativeWindow(
			mref<uptr<platform::NativeWindow>> window_
		) override;

	private:
		static bool eventWatch(ptr<void> self_, ptr<SDL_Event> event_);

		void translateEvent(ref<const SDL_Event> event_);
	};
}
