#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::gfx {
	class macro_novtable TextureLikeObject :
		public InheritBase<TextureLikeObject> {
	public:
		using this_type = TextureLikeObject;

	public:
		constexpr TextureLikeObject() noexcept = default;

		constexpr TextureLikeObject(cref<this_type>) noexcept = delete;

		constexpr TextureLikeObject(mref<this_type>) noexcept = delete;

		constexpr ~TextureLikeObject() noexcept override = default;

	public:
		ref<this_type> operator=(cref<this_type>) noexcept = delete;

		ref<this_type> operator=(mref<this_type>) noexcept = delete;
	};
}
