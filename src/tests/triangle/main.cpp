#include <Common.h>

#include <Instance.h>
#include <Device.h>
#include <SwapChain.h>
#include <Window.h>
#include <Surface.h>
#include <RenderObject.h>
#include <BufferManager.h>
#include <Pipeline.h>
#include <RenderPass.h>
#include <CommandDispatcher.h>
#include <VertexBuffer.h>
#include <DrawCall.h>
#include <IndexBuffer.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

constexpr const char* VertShader = R"Shader(

#version 450


layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
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
	glm::vec2 pos;
	glm::vec3 color;
};

class Triangle : public vkl::RenderObject
{
	REFLECTED_TYPE(Triangle, vkl::RenderObject)
	static void populateReflection(vkl::RenderObjectDescription& reflection)
	{
		reflection.pipelineDescription().setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, VertShader);
		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader);

		reflection.pipelineDescription().declareVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(Vertex), offsetof(Vertex, pos));
		reflection.pipelineDescription().declareVertexAttribute(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(Vertex), offsetof(Vertex, color));
	}

public:
	Triangle(const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, const vkl::PipelineManager& pipelines) : vkl::RenderObject(device, swapChain, bufferManager, pipelines)
	{
		init(device, swapChain, bufferManager, pipelines);  

		auto vbo = bufferManager.createVertexBuffer(device, swapChain);

		_verts.push_back({ glm::vec2(0.0, -0.5), glm::vec3(1,0,0) });
		_verts.push_back({ glm::vec2(-0.5, 0.5), glm::vec3(0,1,0) });
		_verts.push_back({ glm::vec2(0.5, 0.5), glm::vec3(0,0,1) });

		vbo->setData(_verts.data(), sizeof(Vertex), _verts.size());
		addVBO(device, swapChain, vbo, 0);

		auto drawCall = std::make_shared<vkl::DrawCall>();
		drawCall->setCount(3);

		_indices.push_back(0);
		_indices.push_back(1);
		_indices.push_back(2);
		auto indexBuffer = bufferManager.createIndexBuffer(device, swapChain);
		indexBuffer->setData(_indices);
		
		drawCall->setIndexBuffer(indexBuffer);

		addDrawCall(device, swapChain, drawCall);
	}

private:
	std::vector<Vertex> _verts;
	std::vector<uint32_t> _indices;
};

IMPL_REFLECTION(Triangle)

int main(int argc, char* argv[])
{
	vkl::Instance instance("triangle_vkl", true);
	vkl::Window window(1080, 720, "triangle_vkl");
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

	std::vector<std::shared_ptr<vkl::RenderObject>> renderObjects;
	renderObjects.push_back(std::make_shared<Triangle>(device, swapChain, bufferManager, pipelineManager));

	while (!window.shouldClose())
	{
		swapChain.prepNextFrame(device, surface, commandDispatcher, mainPass, window.getWindowSize());
		bufferManager.update(device, swapChain);
		commandDispatcher.processUnsortedObjects(renderObjects, pipelineManager, mainPass, swapChain, swapChain.frameBuffer(swapChain.frame()), swapChain.swapChainExtent());
		swapChain.swap(device, surface, commandDispatcher, mainPass, window.getWindowSize());
		window.pollEvents();
	}

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

	instance.cleanUp();
	vkl::Window::cleanUpWindowSystem();

}