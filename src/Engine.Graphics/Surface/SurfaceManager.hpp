#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::platform {
	class NativeWindow;
}

namespace hg::engine::graphics {
	class Application;
	class Surface;
	class Swapchain;
}

namespace hg::engine::graphics {
	class SurfaceManager {
	public:
		using this_type = SurfaceManager;

	public:
		SurfaceManager(ref<Application> application_);

		~SurfaceManager();

	private:
		void tidy();

	private:
		nmpt<Application> _application;

		StableUnorderedMap<
			nmpt<platform::NativeWindow>, uptr<Surface>
		> _surfaces;

	public:
		/**
		 *
		 * @returns A pointer to the bound surface, otherwise nullptr.
		 */
		[[nodiscard]] nmpt<Surface> getSurface(nmpt<platform::NativeWindow> window_);

		/**
		 *
		 * @returns A pointer to the newly create surface, otherwise nullptr.
		 */
		[[nodiscard]] nmpt<Surface> makeSurface(mref<uptr<platform::NativeWindow>> window_);

	private:
		bool destroySurface(mref<uptr<Surface>> surface_);

	public:
		bool destroySurface(nmpt<platform::NativeWindow> window_);

		bool destroySurface(mref<nmpt<Surface>> surface_);
	};
}
