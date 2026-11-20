#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/__macro.hpp>

#include "ComputePassSpecification.hpp"
#include "GraphicsPassSpecification.hpp"
#include "MeshPassSpecification.hpp"
#include "RaytracingPassSpecification.hpp"

namespace hg::engine::accel::lang::ir {
	struct StageContract;
}

namespace hg::engine::accel {
	class macro_novtable EffectSpecification {
	public:
		using this_type = EffectSpecification;

	protected:
		EffectSpecification() noexcept = default;

	public:
		EffectSpecification (cref<this_type>) = delete;

		EffectSpecification (mref<this_type>) = delete;

		virtual ~EffectSpecification() = default;

	public:
		/**
		 * @details The targeted symbols to export. Primarly used for optimization
		 *  and trimming of the provided effect.
		 */
		[[nodiscard]] virtual cref<Vector<smr<const lang::Symbol>>> targetSymbols() const noexcept = 0;

		/**
		 * @details The bindings the pass provides to every stage and the outputs it requires of the export stage.
		 *  `nullptr`: the pass declares nothing and the stage only checks itself.
		 */
		[[nodiscard]] virtual ptr<const lang::ir::StageContract> stageContract() const noexcept {
			return nullptr;
		}

	public:
		template <typename Type_>
		[[nodiscard]] Type_ getPassSpec() const;

	public:
		[[nodiscard]] virtual ComputePassSpecification getComputeSpec() const = 0;

		[[nodiscard]] virtual GraphicsPassSpecification getGraphicsSpec() const = 0;

		[[nodiscard]] virtual MeshPassSpecification getMeshSpec() const = 0;

		[[nodiscard]] virtual RaytracingPassSpecification getRaytracingSpec() const = 0;
	};

	template <>
	inline GraphicsPassSpecification EffectSpecification::getPassSpec<GraphicsPassSpecification>() const {
		return this->getGraphicsSpec();
	}

	template <>
	inline MeshPassSpecification EffectSpecification::getPassSpec<MeshPassSpecification>() const {
		return this->getMeshSpec();
	}

	template <>
	inline RaytracingPassSpecification EffectSpecification::getPassSpec<RaytracingPassSpecification>() const {
		return this->getRaytracingSpec();
	}

	template <>
	inline ComputePassSpecification EffectSpecification::getPassSpec<ComputePassSpecification>() const {
		return this->getComputeSpec();
	}
}
