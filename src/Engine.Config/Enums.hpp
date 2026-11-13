#pragma once

namespace hg::engine::cfg {
	/**/

	enum class ProjectConfigProperty {
		/**/
		eLocalAssetPath,
		eLocalBasePath,
		eLocalCachePath,
		/**/
		eProjectTitle
	};

	/**/

	enum class EditorConfigProperty {
		/**/
		eLocalAssetPath,
		eLocalCachePath,
		eLocalEditorPath
	};

	/**/

	enum class SchedulerConfigProperty {
		/**/
		eWorkerCount,
		eWorkerReserve
	};

	/**/

	enum class RuntimeConfigProperty {
		/**/
		eLocalBasePath,
		eLocalCachePath
	};
}
