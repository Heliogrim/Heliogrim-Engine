#pragma once

#include "..\IEntity.hpp"

/** A living entity. */
class ILivingEntity :
	public IEntity
{
	/**
	 * Is living
	 *
	 * @return A const bool.
	 */
	bool isLiving() const final override { return true; };
};