#pragma once

#include "Inc.hpp"

#include <Engine.Utils/Generator.hpp>
#include <Engine.Utils/Register.hpp>

#include "./Library/Application.hpp"
#include "./Library/CommandPool.hpp"
#include "./Library/CtrlCamera.hpp"
#include "./Library/MasterRenderer.hpp"
#include "./Library/Offscreen.hpp"
#include "./Library/Queue.hpp"
#include "./Library/Surface.hpp"
#include "./Library/Swapchain.hpp"
#include "Library/DescriptorPool.hpp"
#include "Library/MasterScene.hpp"
#include "Library/RenderCache.hpp"
#include "./Library/HandlerMap/MatMap.hpp"
#include "./Library/HandlerMap/ModelMap.hpp"
#include "./Library/HandlerMap/ObjMap.hpp"
#include "./Library/HandlerMap/TexMap.hpp"
#include "./Library/Texture/TextureArray.hpp"
#include "Skybox/Skybox.hpp"
#include "./Utils/Generator/MipMapGenerator.hpp"
#include "./Utils/Generator/TerrainMeshGenerator.hpp"

#define FENCE_TIMEOUT 100000000

/** . */
namespace clockwork::__internal::core {
	/**
	 * A graphics.
	 *
	 * @author Julius
	 * @date 17.01.2020
	 */
	class GraphicsDep {
	public:
		/** Default constructor */
		GraphicsDep();

		/**
		 * Destructor
		 */
		~GraphicsDep();

		/** Destroys this Graphics */
		void destroy();

		/**
		 * Will render a frame
		 *	If the camera changed, it will invoke a update on the renderers and scenes
		 *	It will also submit the current state of the camera, so any change
		 *		can get detected by next frame
		 */
		void render();

		/**
		 * Setup the graphics
		 *	This will create a new instance and the matching surface.
		 *	Based on that the vulkan api required data will be allocated and adjusted.
		 *	
		 *	This will also create the renderer, their scenes and the players camera
		 */
		void setup();

		/**
		 * \brief 
		 * \return 
		 */
		bool ready() const;

		utils::__Generator generator;

		gfx::types::__ptr__CtrlCamera camera;

	private:
		gfx::types::__ptr__Application _application;

		gfx::types::__Device _device;
		utils::__Register _register;

		HINSTANCE _hInstance;

		gfx::utils::__ptr__TextureInitializer _textureInitializer;

		gfx::utils::__ptr__MipMapGenerator _mipMapGenerator;
		gfx::utils::TerrainMeshGenerator _terrainMeshGenerator;

		gfx::types::__Surface _surface;
		gfx::types::__Swapchain _swapchain;
		gfx::types::__ptr__Offscreen _offscreen;

		gfx::types::CommandPool _commandPool;
		gfx::types::CommandPool _commandPoolTrans;

		inline static std::vector<const char*> _deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		gfx::types::__Queue _graphicQueue;
		gfx::types::__Queue _computeQueue;
		gfx::types::__Queue _transferQueue;

		gfx::types::TexMap _texMap;
		gfx::types::MatMap _matMap;
		gfx::types::ModelMap _modelMap;
		gfx::types::ObjMap _objMap;

	private:
		/**
		 * Descriptor Pool
		 */
		gfx::types::DescriptorPool _descriptorPool;

		void setupDescriptorPool();

	private:
		/**
		 * Renderer
		 */
		gfx::types::MasterRenderer _renderer;

	private:
		/**
		 * Render Cache
		 */
		gfx::types::RenderCache _renderCache;

	private:
		/**
		 * Render Scene
		 */
		gfx::types::MasterScene _renderScene;

	private:
		int HEIGHT = 1080;
		int WIDTH = 1920;

		struct {
			vk::Framebuffer frame;
		} _offScreen;

		struct {
			gfx::types::TextureArray texArray;
			// Attribute Mapping
			gfx::types::TextureArray normalArray;// Normal Mapping - Array
			gfx::types::TextureArray specularArray;// Specular Mapping - Array
			gfx::types::TextureArray bumpArray;// Bump Mapping - Array
		} _scene;

		/**
		 *	### Skybox Data ###
		 */

		graphics::skybox::Skybox _skybox;

		/**
		 *
		 */

		/**
		 * ### Terrain Data ###
		 */

		graphics::terrain::Terrain _terrain;

		/**
		 *
		 */

		/**
		 * Creates class buffer
		 *
		 * @tparam T Generic type parameter.
		 * @param count_ Number of.
		 *
		 * @return The new class buffer.
		 */
		template <class T>
		gfx::types::Buffer createClassBuffer(const uint32_t count_ = 1) const {
			const size64_t size = sizeof(T) * count_;
			gfx::types::Buffer buffer = _device->createBuffer(
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eHostVisible, size);
			buffer.bind();

			return buffer;
		}

		/**
		 * Searches for the best graphic device and creates a new instance
		 *	It will also search for queue's, which are split into graphics, compute and transfer
		 *	If a queue is not available it will shared one and fallback to graphics queue
		 */
		void createDevice();

		/**
		 * Will create helper instances like Loader and TextureInitializer
		 */
		void createHelper();

		/**
		 * Will create queues for devices
		 */
		void createQueues();

		/**
		 * Will create contexts for every thread in graphic thread pool
		 */
		void createContext();

		/** Creates texture array */
		void createTextureArray();

		/**
		 * Sets up the skybox
		 *	Creates a new skybox object
		 *	Stores skybox to skybox renderer's scene
		 */
		void setupSkybox();

		/**
		 * Sets up the terrain
		 *	Creates a new terrain object
		 *	Stores terrain to terrain renderer's scene
		 */
		void setupTerrain();

		#ifdef _DEBUG
		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
		PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = VK_NULL_HANDLE;
		PFN_vkDebugReportMessageEXT DebugReportMessage = VK_NULL_HANDLE;

		VkDebugReportCallbackEXT _debugMessenger = VK_NULL_HANDLE;

		void createDebugCallback();
		#endif
	};

	TYPE_SHARED(GraphicsDep, Graphics);
}
