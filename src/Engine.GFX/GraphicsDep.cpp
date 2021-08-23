#include "GraphicsDep.hpp"

#include <map>
#include <unordered_set>

#include <Engine.Include/Graphic/Texture/Texture.hpp>
#include "./Library/Shadow/CascadedShadow.hpp"
#include "./Library/Swapchain.hpp"
#include "./Library/Object/Object.hpp"
#include "./Utils/Initializer/Texture/VkTextureInitializer.hpp"

#define TEX_ARRAY_DIM 4096

#define TEX_ARRAY_COUNT 1

#define LIGHT_FOV 45.0F
#define ZNEAR 1.0F
#define ZFAR 160.0F

using __Model = clockwork::__internal::gfx::types::__Model;
using __ObjectInterface = clockwork::__internal::gfx::types::__ObjectInterface;
using __Tasking = clockwork::__internal::scheduler::__Tasking;

using namespace clockwork::__internal::core;
using namespace clockwork::__internal::concurrent;

/* Common */

GraphicsDep::GraphicsDep(const __Tasking& tasking_) :
	camera(nullptr),
	_application(nullptr),
	_tasking(tasking_),
	_hInstance(nullptr),
	_textureInitializer(nullptr),
	_mipMapGenerator(nullptr),
	_offscreen(nullptr),
	_renderCache(nullptr) {}

GraphicsDep::~GraphicsDep() {
	// Offscreen
	delete _offscreen;
}

bool GraphicsDep::ready() const {
	return _application != nullptr &&
		camera != nullptr &&
		_register;
	// TODO: Check whether renderer are set-up
}

void GraphicsDep::render() {
	// Check for camera change and submit changes to uniform buffers
	if (camera->changed()) {
		camera->updatePosition();
	}

	// Rendering
	_renderer.invoke();

	// Reset State to capture state change
	camera->resetState();
}

/**
 * \brief
 */
void GraphicsDep::setupSkybox() {
	_skybox = graphics::skybox::Skybox(nullptr);
	/*
	gfx::types::Buffer vertices = _device->createBuffer(
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal, _skybox.model()->vertices.size);
	gfx::types::Buffer indices = _device->createBuffer(
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal, _skybox.model()->indices.size);

	vertices.bind();
	indices.bind();

	_device->copyBuffer(_skybox.model()->vertices, vertices, _device->graphicQueue);
	_device->copyBuffer(_skybox.model()->indices, indices, _device->graphicQueue);

	_skybox.model()->vertices.destroy();
	_skybox.model()->indices.destroy();

	_skybox.model()->vertices = vertices;
	_skybox.model()->indices = indices;

	// TODO: store Skybox
	const gfx::types::CompositeGeometryBuffer cgb {
		{ &_skybox.model()->indices.buffer, UINT32_T(_skybox.model()->indexCount), 0 },
		{},
		{ &_skybox.model()->vertices.buffer, UINT32_T(_skybox.model()->vertexCount), 0 }
	};
	_renderer.mrtRenderer()->skyboxRenderer().geometry() = cgb;

	auto& albedo = _skybox.texture();
	albedo.makeSampler();
	albedo.updateDescriptor();

	_renderer.mrtRenderer()->skyboxRenderer().albedo() = albedo;
	_renderer.mrtRenderer()->skyboxRenderer().bindings().array()[1]->setImageDescriptor(
		const_cast<vk::DescriptorImageInfo*>(&albedo.descriptor()));
	_renderer.mrtRenderer()->skyboxRenderer().bindings().update();
	 */
}

void GraphicsDep::setupTerrain() {
	_terrain = graphics::terrain::Terrain(_device, generator);
	/*
	_terrain.blendmap().makeSampler();
	_terrain.blendmap().updateDescriptor();

	_terrain.samplermap()->update();

	// TODO: store Terrain
	_renderer.mrtRenderer()->terrainRenderer().terrain() = _terrain;

	_renderer.mrtRenderer()->terrainRenderer().bindings().array()[1]->setImageDescriptor(
		const_cast<vk::DescriptorImageInfo*>(&_terrain.blendmap().descriptor()));
	_renderer.mrtRenderer()->terrainRenderer().bindings().array()[2]->setImageDescriptor(
		const_cast<vk::DescriptorImageInfo*>(&_terrain.samplermap()->descriptor()));
	_renderer.mrtRenderer()->terrainRenderer().bindings().update();
	 */
}

void GraphicsDep::setup() {
	// BUG: Disable Steam-Overlay causing crash on queue submit
	//_putenv("DISABLE_VK_LAYER_VALVE_steam_overlay_1=1");

	// Instance
	_application = new gfx::types::Application();
	_application->setup();

	#ifdef _DEBUG
	// Debug Callback
	createDebugCallback();
	#endif

	// Surface
	_surface = types::shared<gfx::types::Surface>(&_application->instance());
	_surface->setup(WIDTH, HEIGHT);

	// Device
	createDevice();
	createQueues();
	createContext();

	// Descriptor Pool
	setupDescriptorPool();

	// Helper
	createHelper();

	// Cache
	_renderCache = clockwork::__internal::gfx::types::RenderCache(_device);
	_renderCache.setup();

	// pre reserve memory in caches
	/*
	_vertexCache->reserve(1 << 19);// ~ 512k
	_indexCache->reserve(1 << 18);// ~ 256k
	_instanceCache->reserve(1 << 15);// ~ 32k
	_materialCache->reserve(1 << 6);// ~ 32
	_objectMetaCache->reserve(1 << 3);// ~ 4
	*/

	// Swapchain
	_swapchain = types::shared<gfx::types::Swapchain>(_device, _surface, _textureInitializer);
	_swapchain->prepare();
	_swapchain->setup();

	// Offscreen
	_offscreen = new gfx::types::Offscreen(_device);
	_offscreen->setup(_textureInitializer);

	// Rendering
	_renderer.setDevice(_device);
	_renderer.setGraphicsQueue(_device->graphicQueue);
	_renderer.setSwapchain(_swapchain);
	_renderer.setup();

	// Camera as reference from master scene
	camera = static_cast<gfx::types::__ptr__CtrlCamera>(_renderScene.camera);
	camera->setPerspective(FOV, static_cast<float>(_swapchain->width()) / static_cast<float>(_swapchain->height()),
		NEAR_PLANE, FAR_PLANE);
	camera->updatePosition();
	camera->updateView();
	gfx::types::SharedCamera::update(camera);

	// TODO: actually brute force there size and auto scale them self at runtime while graphics applies

	// TODO: relocate skybox loading
	setupSkybox();

	// TODO: relocate terrain loading
	setupTerrain();

	// Register
	_register = types::shared<utils::Register>();

	/**
	 * TODO:
	 *	create TextureArray and store it to MasterRenderer ~> ObjectRenderer
	 *			createTextureArray();
	 *
	 *	create Skybox and store it to SkyboxRenderer
	 *			_skybox = Skybox(loader);
	 *
	 *	create Terrain and store it to TerrainRenderer
	 *			_terrain = Terrain();
	 *			_terrain.model() = _terrainMeshGenerator.generateTerrain();
	 *			_terrain.createSamplerMap(_textureInitializer);
	 *			setupTerrainBlend(std::string("blend.ktx"), std::string("back.ktx"), std::string("r.ktx"), std::string("g.ktx"), std::string("b.ktx"));
	 *
	 */
	createTextureArray();
}

void GraphicsDep::createHelper() {
	_textureInitializer = types::ptr<gfx::utils::VkTextureInitializer>(_device);
	generator = types::shared<utils::Generator>(_device, _device->transferQueue, _textureInitializer);
}

/**
 * \brief
 */
void GraphicsDep::destroy() {
	_swapchain->destroy();
	_device->logDev.destroy();
	_surface.reset();

	SDL_Quit();

	_application->destory();
}

/**
 * \brief
 */
void GraphicsDep::createDevice() {
	// Device
	std::vector<vk::PhysicalDevice, std::allocator<vk::PhysicalDevice>> devices = _application
		->instance().
		enumeratePhysicalDevices();
	//QueueFamilyIndices indices;

	for (const auto& device : devices) {
		//indices = QueueFamilyIndices();
		//indices.graphicsFamily = _device->getQueueFamilyIndex(vk::QueueFlagBits::eGraphics);
		//indices.presentFamily = NULL;

		bool swapChainAdequate = false;

		auto extensions = device.enumerateDeviceExtensionProperties(nullptr);
		_STD unordered_set<std::string> requiredExtension(_deviceExtensions.begin(), _deviceExtensions.end());

		for (const auto& extension : extensions) {
			requiredExtension.erase(extension.extensionName);
		}

		const bool extSupport = requiredExtension.empty();

		if (extSupport) {
			vk::SurfaceCapabilitiesKHR caps = device.getSurfaceCapabilitiesKHR(_surface->surface);
			auto formats = device.getSurfaceFormatsKHR(_surface->surface);
			auto modes = device.getSurfacePresentModesKHR(_surface->surface);

			swapChainAdequate = (!formats.empty() && !modes.empty());
		}

		if (extSupport && swapChainAdequate) {
			_device = types::shared<gfx::types::Device>(device);
			DEBUG_SNMSG(true, "VK", "Found vk::PhysicDevice matching requirements.")
			break;
		}
	}

	assert(_device->physDev);
	_device->createLogic(_device->features, _deviceExtensions, _application->validationLayers());
	assert(_device->logDev);
}

/**
 * \brief
 */
void GraphicsDep::createQueues() {
	// Graphic Queue
	_graphicQueue = types::shared<gfx::types::Queue>(_device->logDev);
	_graphicQueue->setup(_device->qFamIndices.graphicsFamily, 0);

	if (_device->qFamIndices.graphicsFamily == _device->qFamIndices.computeFamily)
		_computeQueue = _graphicQueue;

	if (_device->qFamIndices.graphicsFamily == _device->qFamIndices.transferFamily)
		_transferQueue = _graphicQueue;

	// Compute Queue
	if (_computeQueue == nullptr) {
		_computeQueue = types::shared<gfx::types::Queue>(_device->logDev);
		_computeQueue->setup(_device->qFamIndices.computeFamily, 0);
	}

	// Transfer Queue
	if (_transferQueue == nullptr) {
		_transferQueue = types::shared<gfx::types::Queue>(_device->logDev);
		_transferQueue->setup(_device->qFamIndices.transferFamily, 0);
	}

	_device->graphicQueue = _graphicQueue;
	_device->computeQueue = _computeQueue;
	_device->transferQueue = _transferQueue;

	// Validate Queues
	assert(_device->graphicQueue);
	assert(_device->computeQueue);
	assert(_device->transferQueue);
	DEBUG_SNMSG(true, "VK", "Created vk::Queue's successfully.")
}

/**
 * \brief
 */
void GraphicsDep::createContext() {
	_device->createContext();

	std::vector<future<void>> contextGate = std::vector<future<void>>();
	_tasking->each([&]() {
		_device->createContext();
	});

	// for (uint8_t i = 0; i < threadPool->size(); i++)
	// {
	// 	types::promise<void> task = types::promise<void>([&]() { _device->createContext(); });
	// 	types::future<void> result = task.get_future();
	//
	// 	threadPool->add(i, task);
	//
	// 	contextGate.push_back(result);
	// }

	for (const future<void>& entry : contextGate) {
		entry.join();
	}
}

void GraphicsDep::setupDescriptorPool() {
	gfx::types::DescriptorPool::shared_type pool = gfx::types::DescriptorPool::make();

	pool->setup(_device);
}

// TODO: refactor readability ~> e.g. rename
void GraphicsDep::createTextureArray() {
	#ifndef MAX_LAYERS
	#define MAX_LAYERS 16
	#endif

	#ifndef TEX_ARRAY_DIM
#define TEX_ARRAY_DIM 1024
	#endif

	#ifndef TEX_ARRAY_COUNT
#define TEX_ARRAY_COUNT 4
	#endif
	vk::CommandBuffer cmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary,
		_device->graphicQueue->familyIndex(), true);
	cmd.begin(vk::CommandBufferBeginInfo());

	gfx::types::__TextureArray tex = types::shared<gfx::types::TextureArray>(
		_device, TEX_ARRAY_DIM, TEX_ARRAY_DIM, MAX_LAYERS);

	tex->meta().format(COLOR_FORMAT);
	tex->samplerMeta().anisotropy(16.F);

	tex->init(*_textureInitializer);
	tex->toImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal, cmd);

	_device->flushCmdBuffer(cmd, _graphicQueue, true);

	cmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, _device->graphicQueue->familyIndex(), true);
	cmd.begin(vk::CommandBufferBeginInfo {});

	DEBUG_SNMSG(true, "RP", "Creating Diffuse-Map...")
	gfx::types::__TextureArray diffuse = types::shared<gfx::types::TextureArray>(_device, 1024, 1024, 16);
	diffuse->meta().format(COLOR_FORMAT);
	diffuse->init(*_textureInitializer);
	diffuse->toImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal, cmd);
	DEBUG_SNMSG(true, "RP", "Created Diffuse-Map successfully.")

	// TODO: store Diffuse Map

	DEBUG_SNMSG(true, "RP", "Creating Specular-Map...")
	gfx::types::__TextureArray specular = types::shared<gfx::types::TextureArray>(_device, 1024, 1024, 16);
	specular->meta().format(COLOR_FORMAT);
	specular->init(*_textureInitializer);
	diffuse->toImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal, cmd);
	DEBUG_SNMSG(true, "RP", "Created Specular-Map successfully.")

	// TODO: store Specular Map

	DEBUG_SNMSG(true, "RP", "Creating Normal-Map...")
	gfx::types::__TextureArray normal = types::shared<gfx::types::TextureArray>(_device, 1024, 1024, 16);
	normal->meta().format(COLOR_FORMAT);
	normal->init(*_textureInitializer);
	diffuse->toImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal, cmd);
	DEBUG_SNMSG(true, "RP", "Created Normal-Map successfully.")

	// TODO: store Normal Map

	DEBUG_SNMSG(true, "RP", "Creating Alpha-Map...")
	gfx::types::__TextureArray alpha = types::shared<gfx::types::TextureArray>(_device, 1024, 1024, 16);
	alpha->meta().format(COLOR_FORMAT);
	alpha->init(*_textureInitializer);
	diffuse->toImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal, cmd);
	DEBUG_SNMSG(true, "RP", "Created Alpha-Map successfully.")

	// TODO: store Alpha Map

	_device->flushCmdBuffer(cmd, _graphicQueue, true);
}

#if FALSE
  /* Comment: Temporary increasing object id */
static uint32_t dbg_object_id = 0;
__ObjectInterface Graphics::createObject(const __IAsset& asset_, const types::vec3& pos_,
	const types::vec3& rot_, const float scale_) {
	if (!_objMap.containsKey(asset_->getAssetId()))
		loadAsset(asset_);

	const gfx::types::__InstanceHandler sptrih = *_objMap.get(asset_->getAssetId());
	const gfx::types::__Model model = sptrih->model();

	const uint32_t id = (dbg_object_id++);

	const gfx::types::__Object obj = clockwork::__internal::types::shared<gfx::types::Object>(
		id,
		model->partCount(),
		pos_,
		rot_,
		scale_
	);
	sptrih->add(obj);

	return std::dynamic_pointer_cast<gfx::types::ObjectInterface>(obj);
}
#endif


#if FALSE
__Model Graphics::prepareModel(const __IAsset& asset_) {
  // Model Exists ? just get shared_ptr otherwise load it
	if (_modelMap.containsKey(asset_->getModelFile())) {
		return *_modelMap.get(asset_->getModelFile());
	} else {
		// Load Model
		future<gfx::types::__Model> modelFuture = loader->loadModel(asset_);

		modelFuture.join();
		gfx::types::__Model tmp = modelFuture.get();

		_modelMap.add(asset_->getModelFile(), tmp);

		/**
		 * Will store vertices and indices to shader buffers and return it's references
		 */
		tmp->vertices.map();
		tmp->indices.map();

		const types::future<bool> cacheFuture = tasking::async<bool>([&]() mutable {
			const gfx::types::VertexContainer vcc = {
				static_cast<gfx::types::UniformVertex*>(tmp->vertices.mapped),
				tmp->vertexCount
			};
			const gfx::types::IndexContainer icc = {
				static_cast<uint32_t*>(tmp->indices.mapped),
				tmp->indexCount
			};

			const auto vertexRef = _renderCache.vertex().store(vcc);
			tmp->setVertexRef(vertexRef);

			tmp->processIndexOffset();
			const auto indexRef = _renderCache.index().store(icc);
			tmp->setIndexRef(indexRef);

			// Cleanup Stage Buffers
			tmp->indices.destroy();
#ifdef _DEBUG
			DEBUG_NMSG("Dbg", "Cleaning up staged buffer (index) of model '" + asset_->getName() + "'.")
#endif
			tmp->vertices.destroy();
#ifdef _DEBUG
			DEBUG_NMSG("Dbg", "Cleaning up staged buffer (vertex) of model '" + asset_->getName() + "'.")
#endif

			tmp->indices = gfx::types::Buffer();
			tmp->vertices = gfx::types::Buffer();

			return true;
		});

		cacheFuture.join();
		return tmp;
	}
}
#endif

#if FALSE
__Model Graphics::prepareModel(const _STD string& model_) {
  // Model Exists ? just get shared_ptr otherwise load it
	if (_modelMap.containsKey(model_)) {
		return *_modelMap.get(model_);
	} else {
		// Load Model
		types::future<gfx::types::__Model> modelFuture = loader->loadModel(model_);

		modelFuture.join();
		gfx::types::__Model tmp = modelFuture.get();

		_modelMap.add(model_, tmp);

		/**
		 * Will store vertices and indices to shader buffers and return it's references
		 */
		tmp->vertices.map();
		tmp->indices.map();

		const types::future<bool> cacheFuture = tasking::async<bool>([&]() mutable {
			const gfx::types::VertexContainer vc = {
				static_cast<gfx::types::UniformVertex*>(tmp->vertices.mapped),
				tmp->vertexCount
			};
			const gfx::types::IndexContainer ic = {
				static_cast<uint32_t*>(tmp->indices.mapped),
				tmp->indexCount
			};

			const auto vertexRef = _renderCache.vertex().store(vc);
			tmp->setVertexRef(vertexRef);

			tmp->processIndexOffset();
			const auto indexRef = _renderCache.index().store(ic);
			tmp->setIndexRef(indexRef);

			// Cleanup Stage Buffers
			tmp->indices.destroy();
#ifdef _DEBUG
			DEBUG_NMSG("Dbg", "Cleaning up staged buffer (index) of model '" + model_ + "'.")
#endif
			tmp->vertices.destroy();
#ifdef _DEBUG
			DEBUG_NMSG("Dbg", "Cleaning up staged buffer (vertex) of model '" + model_ + "'.")
#endif

			tmp->indices = gfx::types::Buffer();
			tmp->vertices = gfx::types::Buffer();

			return true;
		});

		cacheFuture.join();
		return tmp;
	}
}
#endif

#if FALSE
void Graphics::loadAsset(const __IAsset& asset_) {
  // Material
	std::vector<gfx::types::__Material> material = std::vector<gfx::types::__Material>(asset_->getMaterials().size());

	// Model
	gfx::types::__Model model = prepareModel(asset_);

	// Material Exists ? just get shared_ptr otherwise load it
	const std::vector<std::string> strmat = asset_->getMaterials();

	std::vector<gfx::types::__Material> cm;
	std::vector<std::string> texNames;

	for (uint32_t i = 0; i < strmat.size(); i++) {
		const std::string entry = strmat[i];
		gfx::types::__Material lm;

		if (_matMap.getOrCreate(entry, lm)) {
			const __IMaterial im = _register->materials.getMaterialById(entry);

			(*lm.get()) = *loader->loadMaterial(im).get();

			types::future<void> task = tasking::async<void>([&]() mutable {
				const gfx::types::MaterialContainer container = gfx::types::MaterialContainer(&lm->getUniform(), 1);
				const auto ref = _renderCache.material().store(container);

				lm->setMaterialRef(ref);

				cm.push_back(lm);
				texNames.push_back(im->getTexture());
			});

			task.join();
		}

		material[i] = lm;
	}

	// Texture Exists ? just get shared_ptr otherwise load it
	for (uint32_t i = 0; i < texNames.size(); i++) {
		const std::string entry = texNames[i];
		gfx::types::__TextureReference tr;

		if (!_texMap.getOrCreate(entry, tr)) {
			cm[i]->setTexture(tr);
			continue;
		}

		const __ITexture it = _register->textures.getTextureById(entry);
		types::future<gfx::types::__Texture2d> textureFuture = loader->loadTexture(it);

		textureFuture.join();
		const gfx::types::__Texture2d texture = textureFuture.get();

		//const auto ref = _scene.texArray.storeTexture(static_cast<__TextureRoot>(texture));

		/* TODO: !?!
		tasking::async<void>(
			[i = i, &_materialCache = _materialCache, &_device = _device, &texture = texture, &_renderer = _renderer, &
				tr, &cm, entry]() mutable {
				// TODO: texture currently moved to subscene and will be moved to storage for structure reason
				// TODO: every renderer will get an instance of RenderStorage so textures and models will be shared
				// TODO: capsulation for explicit resources like skybox or stream entry terrain
				(*tr.get()) = *_renderer->objectRenderer().
										  refScene().
										  texturemap()->
										  storeTexture(static_cast<gfx::types::__TextureRoot>(texture)).get();

				// Cleanup Stages Image
				_device->logDev.destroySampler(texture->sampler());
				texture->sampler(gfx::Sampler());
				_device->logDev.destroyImageView(texture->view());
				texture->view(gfx::ImageView());
				texture->image().destroy();
#ifdef _DEBUG
				DEBUG_NMSG("Dbg", "Cleaning up texture staged image '" + entry + "'.")
#endif

				cm[i]->setTexture(tr);
				_materialCache->update(cm[i]->materialReference(), cm[i]->getUniform());
			}).wait();
		 */
	}

	gfx::types::__InstanceHandler handler = types::shared<gfx::types::InstanceHandler>(model);

	// Store InstanceHandler to ObjectMap
	_objMap.add(asset_->getAssetId(), handler);

	// Asynchronous call to store __RenderObject when ready to RenderStorage
	// Cache internally has references to CuStagedBuffers of RenderStorage, which will automatically synchronize in-frame
	_tasking->addCondTask([model = model, material = material]() {
			return model->ready() && !material.empty() && std::find_if(material.begin(), material.end(),
				[](const gfx::types::__Material& entry_) {
					return !entry_->isLoaded();
				}) == material.end();
		}, [asset_, handler, storage = _renderStorage]() mutable {
			storage->addInstanceHandler(asset_, handler);
		});
}
#endif

#ifdef _DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags_,
	VkDebugReportObjectTypeEXT objectType_,
	uint64_t object_,
	size_t location_,
	int32_t messageCode_,
	const char* pLayerPrefix_,
	const char* pMessage_,
	void* pUserData_) {
	DEBUG_NMSG("VKM", (std::string(pMessage_) + std::string(pLayerPrefix_)).c_str());
	return VK_FALSE;
}

void GraphicsDep::createDebugCallback() {
	CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(_application
		->instance().getProcAddr(
			"vkCreateDebugReportCallbackEXT")
	);
	DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(
		_application->instance(), "vkDestroyDebugReportCallbackEXT"));
	DebugReportMessage = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(_application->instance(),
			"vkDebugReportMessageEXT")
	);

	VkDebugReportCallbackCreateInfoEXT ci = {};

	ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	ci.pNext = nullptr;
	ci.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT/* | VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_INFORMATION_BIT_EXT*/ |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT;
	ci.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugCallback;

	assert(!CreateDebugReportCallback(_application->instance(), &ci, nullptr, &_debugMessenger));
	DEBUG_NMSG("DEBUG", "Created PFN_vkDebugReportCallbackEXT successfully.")
}
#endif
