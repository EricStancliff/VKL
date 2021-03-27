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
	mainPassOptions.clearColor = { 0.f, 0.f, 0.f, 1.f };
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
	window.manip.process(window.window, window.cam);
	window.swapChain.prepNextFrame(window.device, window.surface, window.commandDispatcher, window.mainPass, window.window.getWindowSize());
	window.bufferManager.update(window.device, window.swapChain);
	window.commandDispatcher.processUnsortedObjects(window.renderObjects, window.pipelineManager, window.mainPass, window.swapChain, window.swapChain.frameBuffer(window.swapChain.frame()), window.swapChain.swapChainExtent());
	window.swapChain.swap(window.device, window.surface, window.commandDispatcher, window.mainPass, window.window.getWindowSize());
}

int main(int argc, char* argv[])
{
	//one instance
	vkl::Instance instance("model_vkl", true);

	VulkanWindow window = buildWindow(instance, "model_vkl");

	//pending updates to reflection and asset mgmt
	std::shared_ptr<vxt::Model> model = nullptr;


	if (!model)
		return -1;

	auto modelObject = std::make_shared<vxt::ModelRenderObject>();
	modelObject->setModel(window.device, window.swapChain, window.bufferManager, window.pipelineManager, model);
	

	while (!window.window.shouldClose())
	{
		//TODO - fix this
		window.window.clearLastFrame();
		vkl::Window::pollEventsForAllWindows();
		window.window.updateToThisFrame();

		modelObject->update(window.device, window.swapChain, window.cam);
		updateWindow(window);
		window.window.clearLastFrame();
		window.window.updateToThisFrame();
	}
	window.window.cleanUp();
	instance.cleanUp();
	vkl::Window::cleanUpWindowSystem();
}