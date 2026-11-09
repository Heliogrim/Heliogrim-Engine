#pragma once

#include <Engine.Async/Future/Future.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Extent.hpp>
#include <Engine.Core/Module/RootModule.hpp>
#include <atomic>

namespace hg::engine::async {
	class AffineThread;
}

namespace hg::engine::platform {
	class NativeWindow;

	// True while DoDragDrop runs on the platform thread; the platform waker then nudges the drag capture window.
	[[nodiscard]] std::atomic<bool>& dragActive() noexcept;
}

namespace hg::engine {
	class macro_novtable Platform :
		public core::RootModule {
	public:
		using this_type = Platform;

	public:
		Platform() noexcept;

		Platform(cref<this_type>) = delete;

		Platform(mref<this_type>) noexcept = delete;

		virtual ~Platform();

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) noexcept = delete;

	public:
		virtual void tidy() = 0;

		virtual void setup() = 0;

	public:
		[[nodiscard]] virtual ref<async::AffineThread> platformThread() const noexcept = 0;

	public:
		[[nodiscard]] virtual ::hg::Future<uptr<platform::NativeWindow>> makeNativeWindow(
			const string_view title_,
			cref<math::Extent2i32> extent_
		) = 0;

		[[nodiscard]] virtual ::hg::Future<bool> destroyNativeWindow(
			mref<uptr<platform::NativeWindow>> window_
		) = 0;
	};
}
