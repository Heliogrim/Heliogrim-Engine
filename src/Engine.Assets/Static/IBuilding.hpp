#pragma once

#include "../IStaticAsset.hpp"

/** A building. */
class IBuilding :
	public IStaticAsset
{
public:

	/**
	 * Is building
	 *
	 * @return A const bool.
	 */
	bool isBuilding() const final override { return true; };
};