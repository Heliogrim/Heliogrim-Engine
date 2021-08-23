#pragma once

#include "..\IStaticAsset.hpp"

/** A plant. */
class IPlant :
	public IStaticAsset
{
public:

	/**
	 * Is plant
	 *
	 * @return A const bool.
	 */
	bool isPlant() const override { return true; };
};