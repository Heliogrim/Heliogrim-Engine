
# Setup
make_library_project(
		LIB_NAME Engine.Accel.Pass
		MODULE
		LIBRARIES ${PROJECT_NAME}::Engine.Graphics ${PROJECT_NAME}::Engine.Common ${PROJECT_NAME}::vulkan.Module ${PROJECT_NAME}::Engine.Reflect ${PROJECT_NAME}::Engine.Core ${PROJECT_NAME}::Engine.Graphics.Types ${PROJECT_NAME}::Engine.Accel.Lang ${PROJECT_NAME}::Engine.Graphics.RenderGraph ${PROJECT_NAME}::Engine.Accel.Pipeline.Module ${PROJECT_NAME}::Engine.Graphics.Core.Module
)
# shortcut: VkAccelerationPassFactory.cpp and VkGraphicsPass.cpp use Engine.Accel.Pipeline (VkGraphicsPipeline), linked as Engine.Accel.Pipeline.Module because Engine.Accel.Pipeline transitively links Engine.Accel.Pass, drop when the pass factory moves above the pipeline bundle
# shortcut: VkAccelerationPassFactory.cpp and VkGraphicsPass.cpp import Engine.Graphics.Core, linked as Engine.Graphics.Core.Module because Engine.Graphics.Core transitively links Engine.Accel.Pass via Engine.Graphics.Render.Subpass; they call Graphics::get()->getCurrentDevice(), drop when the device is passed in