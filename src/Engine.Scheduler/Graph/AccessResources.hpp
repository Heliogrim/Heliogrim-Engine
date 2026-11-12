#pragma once

#include <type_traits>

/**
 * The closed set of state a stage may declare in its reads and writes.
 *
 * A type is an access resource only through an explicit specialisation of `access_resource` in this file.
 * Specialisations do not inherit, so a derived type (`GameScene`) is rejected and must be declared through its
 * registered root (`SceneBase`). Never register a type together with one of its bases: the two keys would differ
 * and the builder could not see the overlap. Tests may register their own tag types in their translation unit.
 */

namespace hg::engine {
	class Input;
}

namespace hg::engine::acs {
	class Registry;
}

namespace hg::engine::core {
	class Timing;
}

namespace hg::engine::graphics::scene {
	class RenderSceneManager;
}

namespace hg::engine::physics {
	class PhysicsSceneSystem;
}

namespace hg::engine::reflow {
	class WindowManager;
}

namespace hg::engine::scene {
	class SceneBase;
}

namespace hg::engine::scheduler::graph {
	template <typename Type_>
	struct access_resource : std::false_type {};

	template <>
	struct access_resource<::hg::engine::Input> : std::true_type {};

	template <>
	struct access_resource<::hg::engine::acs::Registry> : std::true_type {};

	template <>
	struct access_resource<::hg::engine::core::Timing> : std::true_type {};

	template <>
	struct access_resource<::hg::engine::graphics::scene::RenderSceneManager> : std::true_type {};

	template <>
	struct access_resource<::hg::engine::physics::PhysicsSceneSystem> : std::true_type {};

	template <>
	struct access_resource<::hg::engine::reflow::WindowManager> : std::true_type {};

	template <>
	struct access_resource<::hg::engine::scene::SceneBase> : std::true_type {};

	template <typename Type_>
	concept AccessResource = access_resource<Type_>::value;
}
