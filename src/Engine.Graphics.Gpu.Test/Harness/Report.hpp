#pragma once

#include <filesystem>
#include <functional>
#include <utility>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "Contract.hpp"
#include "Raster.hpp"

namespace hg::test::gpu {
	/**
	 * @details Describes the inputs behind a texel, for example `roughness=0.25 ndh=0.75`.
	 */
	using InputsAt = std::function<String(u32 x_, u32 y_)>;

	struct ErrorSummary {
		f32 maxError;
		math::uivec2 maxAt;
		f32 meanError;
	};

	struct Report {
		String test;
		math::uivec2 extent;
		Vector<CheckResult> checks;
		ErrorSummary vsExpected;
		Raster errorMap;
		Opt<ErrorSummary> vsPrevious;
		u32 changedTexels;
		Vector<std::pair<math::uivec2, String>> inputs;

		// Free text printed ahead of the checks, for example compiler diagnostics
		String notes;

		[[nodiscard]] bool passed() const noexcept;

		[[nodiscard]] String text() const;
	};

	[[nodiscard]] Report makeReport(
		const StringView test_,
		ref<const Raster> actual_,
		ref<const Raster> expected_,
		Vector<CheckResult> checks_,
		ref<const InputsAt> inputs_,
		const ptr<const Raster> previous_
	);
}

namespace hg::test::gpu::store {
	/**
	 * @details `<exe dir>/gpu-results`, created on demand.
	 */
	[[nodiscard]] std::filesystem::path resultDir();

	[[nodiscard]] Opt<Raster> loadPrevious(const StringView test_);

	void savePassing(const StringView test_, ref<const Raster> actual_);

	void writeReport(ref<const Report> report_);
}
