#include <vkl/Common.h>

#include <vkl/Instance.h>
#include <vkl/Device.h>
#include <vkl/SwapChain.h>
#include <vkl/Window.h>
#include <vkl/Surface.h>
#include <vkl/RenderObject.h>
#include <vkl/BufferManager.h>
#include <vkl/Pipeline.h>
#include <vkl/RenderPass.h>
#include <vkl/CommandDispatcher.h>
#include <vkl/VertexBuffer.h>
#include <vkl/DrawCall.h>
#include <vkl/IndexBuffer.h>
#include <vxt/LinearAlgebra.h>
#include <vxt/FirstPersonManip.h>
#include <vxt/Camera.h>
#include <vxt/Model.h>
#include <vxt/ModelRenderObject.h>

constexpr const char* VertShader = R"Shader(

#version 450

layout(push_constant) uniform MVP {
	mat4 mvp;
} u_mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = u_mvp.mvp * vec4(inPosition, 1.0);
    fragColor = inColor;
}
)Shader";
constexpr const char* FragShader = R"Shader(

#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)Shader";

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
};

class Axis : public vkl::RenderObject
{
	PIPELINE_TYPE
	
	static void describePipeline(vkl::PipelineDescription& description)
	{
		description.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

		description.addShaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, VertShader);
		description.addShaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader);

		description.declareVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Vertex), offsetof(Vertex, pos));
		description.declareVertexAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Vertex), offsetof(Vertex, color));

		description.declarePushConstant(sizeof(glm::mat4));
	}

public:
	Axis(const vkl::Device& device, const vkl::SwapChain& swapChain, const vkl::PipelineManager& pipelines, vkl::BufferManager& bufferManager)
	{
		auto vbo = bufferManager.createVertexBuffer(device, swapChain);

		//x
		_verts.push_back({ glm::vec3(0.0, 0.0, 0.), glm::vec3(1,0,0) });
		_verts.push_back({ glm::vec3(100.0, 0.0, 0.), glm::vec3(1,0,0) });

		//y
		_verts.push_back({ glm::vec3(0.0, 0.0, 0.), glm::vec3(0,1,0) });
		_verts.push_back({ glm::vec3(0.0, 100.0, 0.), glm::vec3(0,1,0) });

		//z
		_verts.push_back({ glm::vec3(0.0, 0.0, 0.), glm::vec3(0,0,1) });
		_verts.push_back({ glm::vec3(0.0, 0.0, 100.0), glm::vec3(0,0,1) });


		vbo->setData(_verts.data(), sizeof(Vertex), _verts.size());
		addVBO(vbo, 0);

		auto drawCall = std::make_shared<vkl::DrawCall>();
		drawCall->setCount(_verts.size());

		addDrawCall(drawCall);

		_mvp = std::make_shared<vkl::PushConstant<glm::mat4>>();
		setPushConstant(_mvp);
	}

	void update(const vxt::Camera& cam)
	{
		_mvp->setData(cam.projection() * cam.view());
	}

private:
	std::vector<Vertex> _verts;
	std::shared_ptr<vkl::PushConstant<glm::mat4>> _mvp;
};

REGISTER_PIPELINE(Axis, Axis::describePipeline)

struct VulkanWindow
{
	vkl::Window window;
	vkl::Surface surface;
	vkl::Device device;
	vkl::SwapChain swapChain;
	vkl::RenderPass mainPass;
	vkl::BufferManager bufferManager;
	vkl::PipelineManager pipelineManager;  
	vkl::CommandDispatcher commandDispatcher;
	std::vector<std::shared_ptr<vkl::RenderObject>> renderObjects;
	vxt::FirstPersonManip manip;
	vxt::Camera cam;

	void cleanUp(const vkl::Instance& instance)
	{
		device.waitIdle();
		for (auto&& ro : renderObjects)
			ro->cleanUp(device);
		renderObjects.clear();
		commandDispatcher.cleanUp(device);
		pipelineManager.cleanUp(device);
		bufferManager.cleanUp(device);
		mainPass.cleanUp(device);
		swapChain.cleanUp(device);
		device.cleanUp();
		surface.cleanUp(instance);
		window.cleanUp();
	}
};

VulkanWindow buildWindow(const vkl::Instance& instance, const std::string& title)
{
	vkl::Window window(1080, 720, title.c_str());
	vkl::Surface surface(instance, window);
	vkl::Device device(instance, surface);

	vkl::SwapChainOptions swapChainOptions{};
	swapChainOptions.swapChainExtent.width = window.getWindowSize().width;
	swapChainOptions.swapChainExtent.height = window.getWindowSize().height;
	vkl::SwapChain swapChain(device, surface, swapChainOptions);

	vkl::RenderPassOptions mainPassOptions;
	mainPassOptions.clearColor = { 0.2f, 0.2f, 0.2f, 1.f };
	vkl::RenderPass mainPass(device, swapChain, mainPassOptions);

	swapChain.registerRenderPass(device, mainPass);

	vkl::BufferManager bufferManager(device, swapChain);
	vkl::PipelineManager pipelineManager(device, swapChain, mainPass);

	vkl::CommandDispatcher commandDispatcher(device, swapChain);

	return {std::move(window), std::move(surface), std::move(device), std::move(swapChain),
		std::move(mainPass), std::move(bufferManager), std::move(pipelineManager), std::move(commandDispatcher),
		{}
	};
}

void updateWindow(VulkanWindow& window)
{
	window.swapChain.prepNextFrame(window.device, window.surface, window.commandDispatcher, window.mainPass, window.window.getWindowSize());
	window.bufferManager.update(window.device, window.swapChain);
	window.commandDispatcher.processUnsortedObjects(window.renderObjects, window.device, window.pipelineManager, window.mainPass, window.swapChain, window.swapChain.frameBuffer(window.swapChain.frame()), window.swapChain.swapChainExtent());
	window.swapChain.swap(window.device, window.surface, window.commandDispatcher, window.mainPass, window.window.getWindowSize());
}

int main(int argc, char* argv[])
{
	//one instance
	vkl::Instance instance("model_vkl", true);

	VulkanWindow window = buildWindow(instance, "model_vkl");

	window.manip.setSpeed(.1f);
	window.manip.enableHeadlight(0);

	vxt::AssetFactory::instance().addSearchPath((std::filesystem::path(VKL_DATA_DIR) / "models").make_preferred().string(), false);

	auto model = vxt::AssetFactory::instance().deviceAsset<vxt::Model>("CesiumMan.glb", window.device, window.swapChain, window.bufferManager);
	//auto model = vxt::AssetFactory::instance().deviceAsset<vxt::Model>("E:\\welderman.glb", window.device, window.swapChain, window.bufferManager);

	if (!model)
		return -1;

	auto modelObject = std::make_shared<vxt::ModelRenderObject>();
  	modelObject->setModel(window.device, window.swapChain, window.bufferManager, window.pipelineManager, model);
	for (auto&& shape : modelObject->shapes())
		window.renderObjects.push_back(shape);

	auto axis = std::make_shared<Axis>(window.device, window.swapChain, window.pipelineManager, window.bufferManager);
	window.renderObjects.push_back(axis);

	window.cam.setView(glm::lookAt(glm::vec3( 5.f,0.f,0.f ), glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 0.f, 1.f, 0.f )));

	while (!window.window.shouldClose())
	{
		//TODO - fix this
		window.window.clearLastFrame();
		vkl::Window::pollEventsForAllWindows();

		window.manip.process(window.window, window.cam);
		uint64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		double seconds = (double)millis / 1000.f;
		modelObject->animate("", seconds);
		modelObject->update(window.device, window.swapChain, window.cam);
		axis->update(window.cam);
		updateWindow(window);
	}
	window.cleanUp(instance);
	instance.cleanUp();
	vkl::Window::cleanUpWindowSystem();
}