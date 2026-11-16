#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow {
	class Widget;

	/**
	 * Phase which has to rerun when an attribute changes; bits combine into masks.
	 */
	enum class ReflowPhase : u8 {
		eLayout = 0b01,
		ePaint = 0b10
	};

	/* Marks the phases on the widget, or defers them while a batch on it is open; nmpt stays mutable, its const reaches the widget. */
	void notifyAttributeChange(nmpt<Widget> widget_, const Flag<ReflowPhase, u8> phases_) noexcept;

	/* Turns a phase mask into pending marks, a layout mark covering paint; an empty mask marks nothing. */
	void markPhases(ref<Widget> widget_, const Flag<ReflowPhase, u8> phases_) noexcept;
}
