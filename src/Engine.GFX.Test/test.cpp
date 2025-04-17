#include "pch.h"
/**/

#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.GFX.Scene/View/SceneViewEye.hpp>
#include <Engine.Render.Scene.Model/CameraModel.hpp>

using namespace hg;

namespace GfxModule {
	namespace Scene {
		TEST(SceneView, StraightDirectionZ) {

			constexpr auto cameraLocation = math::fvec3 { 0.F, 0.F, -5.F };
			constexpr auto cameraRotation = math::Rotator {};

			auto sceneEye = engine::gfx::scene::SceneViewEye {};
			sceneEye.updateView(math::Location { cameraLocation }, cameraRotation);
			sceneEye.updateProjection(math::perspective(glm::radians(70.F), 1.0F, 0.01F, 100.F));

			/**/

			const auto data = math::Location { 0.F, 0.F, 0.F };
			const auto expectedView = math::fvec4 { 0.F, 0.F, 5.F, 1.F };

			const auto viewResult = sceneEye.universeToView(data);
			EXPECT_EQ(viewResult, expectedView);

			const auto projectResult = sceneEye.universeToViewProjected(data);
			EXPECT_EQ(projectResult.x, 0.F);
			EXPECT_EQ(projectResult.y, 0.F);
			EXPECT_GT(projectResult.z, .5F);
			EXPECT_EQ(projectResult.w, 5.F);
		}

		TEST(SceneView, StraightDirectionY) {

			constexpr auto cameraLocation = math::fvec3 { 0.F, 0.F, -5.F };
			constexpr auto cameraRotation = math::Rotator {};

			auto sceneEye = engine::gfx::scene::SceneViewEye {};
			sceneEye.updateView(math::Location { cameraLocation }, cameraRotation);
			sceneEye.updateProjection(math::perspective(glm::radians(70.F), 1.0F, 0.01F, 100.F));

			/**/

			const auto data = math::Location { 0.F, 5.F, 0.F };
			const auto expectedView = math::fvec4 { 0.F, 5.F, 5.F, 1.F };

			const auto viewResult = sceneEye.universeToView(data);
			EXPECT_EQ(viewResult, expectedView);

			const auto projectResult = sceneEye.universeToViewProjected(data);
			EXPECT_EQ(projectResult.x, 0.F);
			EXPECT_NEAR(projectResult.y, -7.1F, 0.1F);
			EXPECT_GT(projectResult.z, .5F);
			EXPECT_EQ(projectResult.w, 5.F);
		}

		TEST(SceneView, StraightDirectionX) {

			constexpr auto cameraLocation = math::fvec3 { 0.F, 0.F, -5.F };
			constexpr auto cameraRotation = math::Rotator {};

			auto sceneEye = engine::gfx::scene::SceneViewEye {};
			sceneEye.updateView(math::Location { cameraLocation }, cameraRotation);
			sceneEye.updateProjection(math::perspective(glm::radians(70.F), 1.0F, 0.01F, 100.F));

			/**/

			const auto data = math::Location { 5.F, 0.F, 0.F };
			const auto expectedView = math::fvec4 { 5.F, 0.F, 5.F, 1.F };

			const auto viewResult = sceneEye.universeToView(data);
			EXPECT_EQ(viewResult, expectedView);

			const auto projectResult = sceneEye.universeToViewProjected(data);
			EXPECT_NEAR(projectResult.x, 7.1F, 1.F);
			EXPECT_EQ(projectResult.y, 0.F);
			EXPECT_GT(projectResult.z, .5F);
			EXPECT_EQ(projectResult.w, 5.F);
		}

		TEST(SceneView, ReconstructOrigin) {

			constexpr auto cameraLocation = math::fvec3 { 0.F, 0.F, -5.F };
			constexpr auto cameraRotation = math::Rotator {};

			auto sceneEye = engine::gfx::scene::SceneViewEye {};
			sceneEye.updateView(math::Location { cameraLocation }, cameraRotation);
			sceneEye.updateProjection(math::perspective(glm::radians(70.F), 1.0F, 0.01F, 100.F));

			/**/

			const auto result = sceneEye.getViewMatrix().inverse()[3].xyz();
			EXPECT_EQ(cameraLocation, result);
		}
	}
}
