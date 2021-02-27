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

#include <PNGLoader.h>

constexpr const char* VertShader = R"Shader(

#version 450


layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragUV = inUV;
}
)Shader";
constexpr const char* FragShader = R"Shader(

#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragUV);
}
)Shader";

struct Vertex
{
	glm::vec2 pos;
	glm::vec2 uv;
};

class ImagePlane : public vkl::RenderObject
{
	REFLECTED_TYPE(ImagePlane, vkl::RenderObject)
	static void populateReflection(vkl::RenderObjectDescription& reflection)
	{
		reflection.pipelineDescription().setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_VERTEX_BIT, VertShader);
		reflection.pipelineDescription().addShaderGLSL(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader);

		reflection.pipelineDescription().declareVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(Vertex), offsetof(Vertex, pos));
		reflection.pipelineDescription().declareVertexAttribute(0, 1, VK_FORMAT_R32G32_SFLOAT, sizeof(Vertex), offsetof(Vertex, uv));

		reflection.pipelineDescription().declareTexture(1);
	}

public:
	ImagePlane(const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager, const vkl::PipelineManager& pipelines) : vkl::RenderObject(device, swapChain, bufferManager, pipelines)
	{
		init(device, swapChain, bufferManager, pipelines);  

		auto vbo = bufferManager.createVertexBuffer(device, swapChain);

		_verts.push_back({ glm::vec2(-0.5, -0.5), glm::vec2(0,0) });
		_verts.push_back({ glm::vec2(-0.5, 0.5), glm::vec2(0,1) });
		_verts.push_back({ glm::vec2(0.5, 0.5), glm::vec2(1,1) });
		_verts.push_back({ glm::vec2(0.5, -0.5), glm::vec2(1,0) });

		vbo->setData(_verts.data(), sizeof(Vertex), _verts.size());
		addVBO(device, swapChain, vbo, 0);

		auto drawCall = std::make_shared<vkl::DrawCall>();

		_indices.push_back(0);
		_indices.push_back(1);
		_indices.push_back(3);
		_indices.push_back(3);
		_indices.push_back(1);
		_indices.push_back(2);
		auto indexBuffer = bufferManager.createIndexBuffer(device, swapChain);
		indexBuffer->setData(_indices);
		drawCall->setCount(_indices.size());

		drawCall->setIndexBuffer(indexBuffer);

		addDrawCall(device, swapChain, drawCall);

		_imageData = vkl::loadJPGData((std::filesystem::path(vkl::vklDataDir()) / "textures" / "texture.jpg").make_preferred().string().c_str(), _width, _height, _components);

		if (_imageData)
		{
			auto texBuff = bufferManager.createTextureBuffer(device, swapChain, _imageData, (size_t)_width, (size_t)_height, (size_t)_components);
			addTexture(device, swapChain, texBuff, 1);
		}
		else
		{
			std::cerr << "Couldn't load example image!!!";
		}
	}
	~ImagePlane()
	{
		if(_imageData)
			vkl::freeJPGData(_imageData);
	}
private:
	std::vector<Vertex> _verts;
	std::vector<uint32_t> _indices;
	int _width{ 0 }, _height{ 0 }, _components{ 0 };
	void* _imageData{ nullptr };
};

IMPL_REFLECTION(ImagePlane)

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
	renderObjects.push_back(std::make_shared<ImagePlane>(device, swapChain, bufferManager, pipelineManager));

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