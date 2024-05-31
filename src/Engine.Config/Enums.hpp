#pragma once

namespace hg::engine::cfg {
	/**/

	enum class ProjectConfigProperty {
		/**/
		eLocalBasePath,
		eLocalCachePath,
		/**/
		eProjectTitle
	};

	/**/

	enum class EditorConfigProperty {
		/**/
		eLocalEditorPath,
		eLocalCachePath,
	};

	/**/

	enum class RuntimeConfigProperty {
		/**/
		eLocalBasePath
	};
}
