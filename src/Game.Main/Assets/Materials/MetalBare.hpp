#pragma once
#include "./Inc.hpp"

namespace clockwork::game::asset {
	class MetalBare :
		public Material {
	public:

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 18.06.2020
		 */
		MetalBare() :
			Material() {
			_id = Url("material::MetalBare");
		}
	private:
	};
}