#pragma once

#include "IAsset.hpp"

/** A static asset. */
class IStaticAsset :
	public asset::subject::IAsset
{
public:

	/**
	 * Is static
	 *
	 * @return A const bool.
	 */
	bool isStatic() const override { return true; };
};