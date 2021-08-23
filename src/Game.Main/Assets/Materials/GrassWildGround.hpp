#pragma once
#include "Inc.hpp"

namespace clockwork::game::asset {
	class GrassWildGround :
		public Material {
	public:

		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 18.06.2020
		 */
		GrassWildGround() :
			Material() {
			_id = Url("material::GrassWildGround");
		}
	private:
	};
}
