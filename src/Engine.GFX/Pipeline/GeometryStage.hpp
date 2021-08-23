#pragma once

#include "../Shader/ShaderSlot.hpp"

namespace ember::engine::gfx::pipeline {

	class GeometryStage final {
	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 03.12.2020
		 */
		GeometryStage() noexcept = default;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 01.12.2020
		 */
		~GeometryStage() noexcept = default;

	private:
		/**
		 * Geometry Shader
		 */
		ShaderSlot _shaderSlot = { shader::ShaderType::eGeometry, "" };

	public:
		/**
		 * Gets the shader
		 *
		 * @author Julius
		 * @date 01.12.2020
		 *
		 * @returns A reference to a const Shader.
		 */
		[[nodiscard]] const ShaderSlot& shaderSlot() const noexcept;

		/**
		 * Get the shader
		 *
		 * @author Julius
		 * @date 14.12.2020
		 *
		 * @returns A reference to a ShaderSlot.
		 */
		[[nodiscard]] ShaderSlot& shaderSlot() noexcept;
	};
}
