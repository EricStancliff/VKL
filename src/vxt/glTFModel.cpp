/******************

Parts of this code are heavily borrowed and modified from Sascha Willem's Vulkan glTF-PBR library (MIT License)
https://github.com/SaschaWillems/Vulkan-glTF-PBR

********************/


#include <vxt/Model.h>

#include <vkl/Common.h>

#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include <iostream>

#include <vkl/DrawCall.h>
#include <vkl/TextureBuffer.h>
#include <vkl/BufferManager.h>
#include <vkl/VertexBuffer.h>
#include <vkl/IndexBuffer.h>

#include <vxt/AssetFactory.h>
#include <vxt/VXT_EXPORT.h>

//helpers fro later versions
namespace tinygltf
{

	static inline int32_t GetTypeSizeInBytes(uint32_t ty) {
		if (ty == TINYGLTF_TYPE_SCALAR) {
			return 1;
		}
		else if (ty == TINYGLTF_TYPE_VEC2) {
			return 2;
		}
		else if (ty == TINYGLTF_TYPE_VEC3) {
			return 3;
		}
		else if (ty == TINYGLTF_TYPE_VEC4) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT2) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT3) {
			return 9;
		}
		else if (ty == TINYGLTF_TYPE_MAT4) {
			return 16;
		}
		else {
			// Unknown componenty type
			return -1;
		}
	}

}

namespace vxt
{
	class VXT_EXPORT glTFModelFile : public FileAsset
	{
	public:
		virtual bool processFile(std::string_view file)
		{
			std::filesystem::path path;
			if (!AssetFactory::instance().resolveAbsoloutePath(file, path))
				return false;

			tinygltf::TinyGLTF gltfContext;

			std::string error;
			std::string warning;

			bool binary = false;
			size_t extpos = file.rfind('.', file.length());
			if (extpos != std::string::npos) {
				binary = (file.substr(extpos + 1, file.length() - extpos) == "glb");
			}

			bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, path.string().c_str()) : gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path.string().c_str());

			if (!warning.empty())
			{
				std::cout << "Could not load gltf file: " << warning << std::endl;
			}

			if (!fileLoaded) {
				std::cerr << "Could not load gltf file: " << error << std::endl;
				return false;
			}

			return true;
		}

		tinygltf::Model gltfModel;
	};


	VkSamplerAddressMode getVkWrapMode(int32_t wrapMode)
	{
		switch (wrapMode) {
		case 10497:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case 33071:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case 33648:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		}
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkFilter getVkFilterMode(int32_t filterMode)
	{
		switch (filterMode) {
		case 9728:
			return VK_FILTER_NEAREST;
		case 9729:
			return VK_FILTER_LINEAR;
		case 9984:
			return VK_FILTER_NEAREST;
		case 9985:
			return VK_FILTER_NEAREST;
		case 9986:
			return VK_FILTER_LINEAR;
		case 9987:
			return VK_FILTER_LINEAR;
		}
		return VK_FILTER_LINEAR;
	}

	class VXT_EXPORT glTFModel : public Model
	{
	public:
		glTFModel() = default;
		virtual ~glTFModel()
		{
			for (auto buff : _buffsToDelete)
				delete[] buff;
		}
		glTFModel(const glTFModel&) = delete;
		glTFModel(glTFModel&&) noexcept = default;
		glTFModel& operator=(glTFModel&&) noexcept = default;
		glTFModel& operator=(const glTFModel&) = delete;

		struct NodeTransform
		{
			glm::vec3 trans{ glm::zero<glm::vec3>() };
			glm::quat rot{  };
			glm::vec3 scale{ glm::one<glm::vec3>() };
			glm::mat4 mat{ glm::identity<glm::mat4>() };
		};
		
		struct AnimationChannel {
			enum PathType { TRANSLATION, ROTATION, SCALE, MORPH };
			PathType path;
			int node;
			uint32_t samplerIndex;
		};

		struct AnimationSampler {
			enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
			InterpolationType interpolation;
			std::vector<double> inputs;
			std::vector<glm::vec4> outputsVec4;
		};

		struct Animation {
			std::string name;
			std::vector<AnimationSampler> samplers;
			std::vector<AnimationChannel> channels;
			double start = std::numeric_limits<double>::max();
			double end = std::numeric_limits<double>::min();
		};

		struct Skin {
			std::string name;
			int skeletonRoot = -1;
			std::vector<glm::mat4> inverseBindMatrices;
			std::vector<int> joints;
		};

		bool buildAsset(std::shared_ptr<const FileAsset> fileAsset, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager) override {
			
			auto glAsset = std::dynamic_pointer_cast<const glTFModelFile>(fileAsset);
			if (!glAsset)
				return false;
			
			auto& gltfModel = glAsset->gltfModel;

			_nodeParents.resize(gltfModel.nodes.size(), -1);
			_nodeTransforms.resize(gltfModel.nodes.size(), {});
			_nodeChildren.resize(gltfModel.nodes.size(), {});

			loadTextureSamplers(gltfModel);
			loadTextures(gltfModel, device, swapChain, bufferManager);
			loadMaterials(gltfModel);
			_indexBuffer = bufferManager.createIndexBuffer(device, swapChain);

			if (gltfModel.animations.size() > 0) {
				loadAnimations(gltfModel);
			}
			loadSkins(gltfModel);

			for (auto& scene : gltfModel.scenes)
				for (auto& node : scene.nodes)
					loadNode(-1, gltfModel.nodes[node], node, gltfModel);

			_vertexBuffer = bufferManager.createVertexBuffer(device, swapChain);
			_vertexBuffer->setData(_verts.data(), sizeof(Vertex), _verts.size());

			_indexBuffer->setData(_indices);
		
			return true;
		}

	protected:

		void loadTextures(const tinygltf::Model& gltfModel, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager)
		{
			for (const tinygltf::Texture& tex : gltfModel.textures) {
				const tinygltf::Image& gltfimage = gltfModel.images[tex.source];

				vkl::TextureOptions options{};
				
				if (tex.sampler != -1) {
					options = _texOptions[tex.sampler];
				}

				const unsigned char* buffer = nullptr;
				VkDeviceSize bufferSize = 0;
				if (gltfimage.component == 3) {
					// Most devices don't support RGB only on Vulkan so convert if necessary
					// TODO: Check actual format support and transform only if required
					bufferSize = gltfimage.width * gltfimage.height * 4;
					auto myBuffer = new unsigned char[bufferSize];
					unsigned char* rgba = myBuffer;
					const unsigned char* rgb = &gltfimage.image[0];
					for (int32_t i = 0; i < gltfimage.width * gltfimage.height; ++i) {
						for (int32_t j = 0; j < 3; ++j) {
							rgba[j] = rgb[j];
						}
						rgba += 4;
						rgb += 3;
					}
					buffer = myBuffer;
					_buffsToDelete.push_back(myBuffer);
				}
				else {
					buffer = &gltfimage.image[0];
					bufferSize = gltfimage.image.size();
				}

				//creates static texture in GPU local memory, so no need to preserve the buffer above
				_textureBuffers.emplace_back(std::move(bufferManager.createTextureBuffer(device, swapChain, buffer, gltfimage.width, gltfimage.height, 4)));

			}
		}

		void loadTextureSamplers(const tinygltf::Model& gltfModel)
		{
			for (const tinygltf::Sampler& smpl : gltfModel.samplers) {
				vkl::TextureOptions options{};
				options.minFilter = getVkFilterMode(smpl.minFilter);
				options.magFilter = getVkFilterMode(smpl.magFilter);
				options.addressModeU = getVkWrapMode(smpl.wrapS);
				options.addressModeV = getVkWrapMode(smpl.wrapT);
				options.addressModeW = options.addressModeV;
				_texOptions.push_back(options);
			}
		}


		void loadMaterials(const tinygltf::Model& gltfModel)
		{
			for (const tinygltf::Material& mat : gltfModel.materials) {
				Model::Material material;
				if (auto find = mat.values.find("baseColorTexture"); find != mat.values.end()) {
					material.baseColorTexture = _textureBuffers[find->second.TextureIndex()];
					//material.texCoordSets.baseColor = find->second.TextureTexCoord();
				}
				if (auto find = mat.values.find("metallicRoughnessTexture"); find != mat.values.end()) {
					material.metallicRoughnessTexture = _textureBuffers[find->second.TextureIndex()];
					//material.texCoordSets.metallicRoughness = find->second.TextureTexCoord();
				}
				if (auto find = mat.values.find("roughnessFactor"); find != mat.values.end()) {
					material.roughnessFactor = static_cast<float>(find->second.Factor());
				}
				if (auto find = mat.values.find("metallicFactor"); find != mat.values.end()) {
					material.metallicFactor = static_cast<float>(find->second.Factor());
				}
				if (auto find = mat.values.find("baseColorFactor"); find != mat.values.end()) {
					material.baseColorFactor = glm::make_vec4(find->second.ColorFactor().data());
				}
				if (auto find = mat.additionalValues.find("normalTexture"); find != mat.additionalValues.end()) {
					material.normalTexture = _textureBuffers[find->second.TextureIndex()];
					//material.texCoordSets.normal = find->second.TextureTexCoord();
				}
				//if (auto find = mat.additionalValues.find("emissiveTexture"); find != mat.additionalValues.end()) {
				//	material.emissiveTexture = _textureBuffers[find->second.TextureIndex()];
				//	material.texCoordSets.emissive = find->second.TextureTexCoord();
				//}
				//if (auto find = mat.additionalValues.find("occlusionTexture"); find != mat.additionalValues.end()) {
				//	material.occlusionTexture = _textureBuffers[find->second.TextureIndex()];
				//	material.texCoordSets.occlusion = find->second.TextureTexCoord();
				//}
				if (auto find = mat.additionalValues.find("alphaMode"); find != mat.additionalValues.end()) {
					const tinygltf::Parameter& param = find->second;
					if (param.string_value == "BLEND") {
						material.alphaMode = Material::ALPHAMODE_BLEND;
					}
					if (param.string_value == "MASK") {
						material.alphaCutoff = 0.5f;
						material.alphaMode = Material::ALPHAMODE_MASK;
					}
				}
				if (auto find = mat.additionalValues.find("alphaCutoff"); find != mat.additionalValues.end()) {
					material.alphaCutoff = static_cast<float>(find->second.Factor());
				}
				//if (auto find = mat.additionalValues.find("emissiveFactor"); find != mat.additionalValues.end()) {
				//	material.emissiveFactor = glm::vec4(glm::make_vec3(find->second.ColorFactor().data()), 1.0);
				//	material.emissiveFactor = glm::vec4(0.0f);
				//}

				//// Extensions
				//// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
				//if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
				//	auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				//	if (ext->second.Has("specularGlossinessTexture")) {
				//		auto index = ext->second.Get("specularGlossinessTexture").Get("index");
				//		material.extension.specularGlossinessTexture = _textureBuffers[index.Get<int>()];
				//		auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
				//		material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
				//		material.pbrWorkflows.specularGlossiness = true;
				//	}
				//	if (ext->second.Has("diffuseTexture")) {
				//		auto index = ext->second.Get("diffuseTexture").Get("index");
				//		material.extension.diffuseTexture = _textureBuffers[index.Get<int>()];
				//	}
				//	if (ext->second.Has("diffuseFactor")) {
				//		auto factor = ext->second.Get("diffuseFactor");
				//		for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
				//			auto val = factor.Get(i);
				//			material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
				//		}
				//	}
				//	if (ext->second.Has("specularFactor")) {
				//		auto factor = ext->second.Get("specularFactor");
				//		for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
				//			auto val = factor.Get(i);
				//			material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
				//		}
				//	}
				//}

				_materials.emplace_back(std::move(material));
			}
		}

		void loadAnimations(const tinygltf::Model& gltfModel)
		{
			for (const tinygltf::Animation& anim : gltfModel.animations) {
				Animation animation{};
				animation.name = anim.name;
				if (anim.name.empty()) {
					animation.name = std::to_string(_animations.size());
				}

				// Samplers
				for (auto& samp : anim.samplers) {
					AnimationSampler sampler{};

					if (samp.interpolation == "LINEAR") {
						sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
					}
					if (samp.interpolation == "STEP") {
						sampler.interpolation = AnimationSampler::InterpolationType::STEP;
					}
					if (samp.interpolation == "CUBICSPLINE") {
						sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
					}

					// Read sampler input time values
					{
						const tinygltf::Accessor& accessor = gltfModel.accessors[samp.input];
						const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
						const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

						assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

						const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
						const float* buf = static_cast<const float*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							sampler.inputs.push_back(buf[index]);
						}

						for (auto input : sampler.inputs) {
							if (input < animation.start) {
								animation.start = input;
							};
							if (input > animation.end) {
								animation.end = input;
							}
						}
					}

					// Read sampler output T/R/S values 
					{
						const tinygltf::Accessor& accessor = gltfModel.accessors[samp.output];
						const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
						const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

						assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

						const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

						switch (accessor.type) {
						case TINYGLTF_TYPE_VEC3: {
							const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
							}
							break;
						}
						case TINYGLTF_TYPE_VEC4: {
							const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								sampler.outputsVec4.push_back(buf[index]);
							}
							break;
						}
						default: {
							std::cout << "unknown type" << std::endl;
							break;
						}
						}
					}

					animation.samplers.push_back(sampler);
				}

				// Channels
				for (auto& source : anim.channels) {
					AnimationChannel channel{};

					if (source.target_path == "rotation") {
						channel.path = AnimationChannel::PathType::ROTATION;
					}
					if (source.target_path == "translation") {
						channel.path = AnimationChannel::PathType::TRANSLATION;
					}
					if (source.target_path == "scale") {
						channel.path = AnimationChannel::PathType::SCALE;
					}
					if (source.target_path == "weights") {
						std::cout << "weights not yet supported, skipping channel" << std::endl;
						continue;
					}
					channel.samplerIndex = source.sampler;
					channel.node = source.target_node;
					if (!channel.node) {
						continue;
					}

					animation.channels.push_back(channel);
				}

				_animations.push_back(animation);
			}
		}

		void loadSkins(const tinygltf::Model& gltfModel)
		{
			for (const tinygltf::Skin& source : gltfModel.skins) {
				Skin newSkin;
				newSkin.name = source.name;

				// Find skeleton root node
				if (source.skeleton > -1) {
					newSkin.skeletonRoot = source.skeleton;
				}

				// Find joint nodes
				for (int jointIndex : source.joints) {
					newSkin.joints.push_back(jointIndex);
				}

				// Get inverse bind matrices from buffer
				if (source.inverseBindMatrices > -1) {
					const tinygltf::Accessor& accessor = gltfModel.accessors[source.inverseBindMatrices];
					const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
					newSkin.inverseBindMatrices.resize(accessor.count);
					memcpy(newSkin.inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
				}

				_skins.emplace_back(std::move(newSkin));
			}
		}

		void loadNode(int parent, const tinygltf::Node& node, int nodeIndex, const tinygltf::Model& model)
		{

			_nodeParents[nodeIndex] = parent;
			_nodeChildren[nodeIndex] = node.children;

			// Generate local node matrix
			if (node.translation.size() == 3) {
				glm::vec3 translation = glm::make_vec3(node.translation.data());
				_nodeTransforms[nodeIndex].trans = translation;
			}

			if (node.rotation.size() == 4) {
				glm::quat q = glm::make_quat(node.rotation.data());
				_nodeTransforms[nodeIndex].rot = glm::mat4(q);
			}

			if (node.scale.size() == 3) {
				glm::vec3 scale = glm::make_vec3(node.scale.data());
				_nodeTransforms[nodeIndex].scale = scale;
			}
			if (node.matrix.size() == 16) {
				_nodeTransforms[nodeIndex].mat = glm::make_mat4x4(node.matrix.data());
			};

			// Node with children
			if (node.children.size() > 0) {
				for (size_t i = 0; i < node.children.size(); i++) {
					loadNode(nodeIndex, model.nodes[node.children[i]], node.children[i], model);
				}
			}

			// Node contains mesh data
			if (node.mesh > -1) {
				const tinygltf::Mesh mesh = model.meshes[node.mesh];
				for (size_t j = 0; j < mesh.primitives.size(); j++) {
					const tinygltf::Primitive& primitive = mesh.primitives[j];
					uint32_t indexStart = static_cast<uint32_t>(_indices.size());
					uint32_t vertexStart = static_cast<uint32_t>(_verts.size());
					uint32_t indexCount = 0;
					uint32_t vertexCount = 0;
					glm::vec3 posMin{};
					glm::vec3 posMax{};
					bool hasSkin = false;
					bool hasIndices = primitive.indices > -1;
					// Vertices
					{
						const float* bufferPos = nullptr;
						const float* bufferNormals = nullptr;
						const float* bufferTexCoordSet0 = nullptr;
						const float* bufferTexCoordSet1 = nullptr;
						const uint16_t* bufferJoints = nullptr;
						const float* bufferWeights = nullptr;

						int posByteStride;
						int normByteStride;
						int uv0ByteStride;
						int uv1ByteStride;
						int jointByteStride;
						int weightByteStride;

						// Position attribute is required
						assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

						const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
						bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
						posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
						posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
						vertexCount = static_cast<uint32_t>(posAccessor.count);
						posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

						if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
							const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
							const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
							bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
							normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
						}

						if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
							const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
							const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
							bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
							uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
						}
						if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
							const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
							const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
							bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
							uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
						}

						// Skinning
						// Joints
						if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
							const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
							const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
							bufferJoints = reinterpret_cast<const uint16_t*>(&(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));
							jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / sizeof(bufferJoints[0])) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
						}

						if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
							const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
							const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
							bufferWeights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
							weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
						}

						hasSkin = (bufferJoints && bufferWeights);

						for (size_t v = 0; v < posAccessor.count; v++) {
							Vertex vert{};
							vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
							vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
							vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
							vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);

							vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * jointByteStride])) : glm::vec4(0.0f);
							vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
							// Fix for all zero weights
							if (glm::length(vert.weight0) == 0.0f) {
								vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
							}
							_verts.push_back(vert);
						}
					}
					// Indices
					if (hasIndices)
					{
						const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
						const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
						const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

						indexCount = static_cast<uint32_t>(accessor.count);
						const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

						switch (accessor.componentType) {
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
							const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
							const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
							const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								_indices.push_back(buf[index] + vertexStart);
							}
							break;
						}
						default:
							std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
							return;
						}
					}

					Model::Primitive prim;
					auto drawCall = std::make_shared<vkl::DrawCall>();
					drawCall->setCount(indexCount);
					drawCall->setOffset(indexStart);
					drawCall->setIndexBuffer(_indexBuffer);
					prim.draw = drawCall;
					prim.material = primitive.material;
					prim.transform = getMatrix(_nodeTransforms, nodeIndex);
					_primitives.emplace_back(std::move(prim));
					_primNodes.push_back(nodeIndex);
				}
			}
		}






		//API
		virtual std::span<const Vertex> getVerts() const override
		{
			return _verts;
		}
		virtual std::shared_ptr<const vkl::VertexBuffer> getVertexBuffer() const override
		{
			return _vertexBuffer;
		}

		virtual std::span<const uint32_t> getIndices() const  override
		{
			return _indices;
		}

		virtual std::shared_ptr<const vkl::IndexBuffer> getIndexBuffer() const  override
		{
			return _indexBuffer;
		}

		virtual std::span<const Primitive> getPrimitives() const  override
		{
			return _primitives;
		}

		virtual std::span<const Material> getMaterials() const override
		{
			return _materials;
		}

		bool supportsAnimations() const override { return !_animations.empty(); }
		bool supportsMorphTargets() const override { return false; }

		bool animate(JointArray& joints, float& jointCount, glm::mat4& shapeTransform, size_t shape, std::string_view animationName, double input, bool loop) const override
		{
			//find my animation
			if (_animations.empty()) {
				return false;
			}

			const Animation* animation = nullptr;
			if (animationName.empty())
			{
				animation = &_animations[0];
			}
			else
			{
				auto findAnimation = std::find_if(_animations.begin(), _animations.end(), [&](const auto& inner) {
					return inner.name == animationName;
					});
				if (findAnimation == _animations.end())
					return false;
				animation = &(*findAnimation);
			}

			if (!animation)
				return false;

			//find my skeleton
			if (shape > _primNodes.size())
				return false;

			auto shapeNode = _primNodes[shape];

			const Skin* skin = findSkin(rootNode(shapeNode)); // we assume that the shape node is somewhere in the same heirarchy as the skin node 
			
			if (!skin)
				return false;

			std::vector<NodeTransform> nodeTransforms = _nodeTransforms;

			if (loop)
			{
				double delta = animation->end - animation->start;
				input = animation->start + fmod(input, delta);
			}

			bool updated = false;
			for (auto& channel : animation->channels) {
				const auto& sampler = animation->samplers[channel.samplerIndex];
				if (sampler.inputs.size() > sampler.outputsVec4.size()) {
					continue;
				}
				for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
					if ((input >= sampler.inputs[i]) && (input <= sampler.inputs[i + 1])) {
						double u = std::max(0.0, input - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
						if (u <= 1.0f) {
							switch (channel.path) {
							case AnimationChannel::PathType::TRANSLATION: {
								glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
								nodeTransforms[channel.node].trans= glm::vec3(trans);
								break;
							}
							case AnimationChannel::PathType::SCALE: {
								glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
								nodeTransforms[channel.node].scale = glm::vec3(trans);
								break;
							}
							case AnimationChannel::PathType::ROTATION: {
								glm::quat q1;
								q1.x = sampler.outputsVec4[i].x;
								q1.y = sampler.outputsVec4[i].y;
								q1.z = sampler.outputsVec4[i].z;
								q1.w = sampler.outputsVec4[i].w;
								glm::quat q2;
								q2.x = sampler.outputsVec4[i + 1].x;
								q2.y = sampler.outputsVec4[i + 1].y;
								q2.z = sampler.outputsVec4[i + 1].z;
								q2.w = sampler.outputsVec4[i + 1].w;
								nodeTransforms[channel.node].rot = glm::normalize(glm::slerp(q1, q2, (float)u));
								break;
							}
							}
							updated = true;
						}
					}
				}
			}

			//update

			if (updated)
			{
				glm::mat4 inverseTransform = glm::inverse(getMatrix(nodeTransforms, shapeNode));
				size_t numJoints = std::min(skin->joints.size(), MaxNumJoints);
				for (size_t i = 0; i < numJoints; i++) {
					int jointNode = skin->joints[i];
					glm::mat4 jointMat = getMatrix(nodeTransforms, jointNode) * skin->inverseBindMatrices[i];
					jointMat = inverseTransform * jointMat;
					joints[i] = jointMat;
				}
				jointCount = (float)numJoints;
				shapeTransform = getMatrix(nodeTransforms, shapeNode);
			}
			return updated;
		}

		glm::mat4 toMatrix(const NodeTransform& transform) const
		{
			return glm::translate(glm::mat4(1.0f), transform.trans) * glm::mat4(transform.rot) * glm::scale(glm::mat4(1.0f), transform.scale) * transform.mat;
		}

		int rootNode(int node) const
		{
			if (node > -1)
			{
				int parent = _nodeParents[node];
				if (parent == -1)
					return node;
				return rootNode(parent);
			}
			else return -1;
		}

		glm::mat4 localMatrix(std::vector<NodeTransform>& transforms, int node) const {
			return toMatrix(transforms[node]);
		}

		glm::mat4 getMatrix(std::vector<NodeTransform>& transforms, int node) const {
			glm::mat4 m = localMatrix(transforms, node);
			int p = _nodeParents[node];
			while (p>-1) {
				m = localMatrix(transforms, p) * m;
				p = _nodeParents[p];
			}
			return m;
		}

		const Skin* findSkin(int node) const
		{
			const Skin* skin = nullptr;
			for (auto&& skin : _skins)
			{
				int skinNode = skin.skeletonRoot;
				if (node == skinNode)
					return &skin;
			}

			for (auto child : _nodeChildren[node])
			{
				skin = findSkin(child);
				if (skin)
					return skin;
			}
			return nullptr;
		}

	private:


		//external
		std::shared_ptr<vkl::VertexBuffer> _vertexBuffer;
		std::shared_ptr<vkl::IndexBuffer> _indexBuffer;
		std::vector<std::shared_ptr<vkl::TextureBuffer>> _textureBuffers;
		std::vector<Model::Material> _materials;
		std::vector<Model::Primitive> _primitives;

		//internal
		std::vector<Model::Vertex> _verts;
		std::vector<uint32_t> _indices;

		std::vector<vkl::TextureOptions> _texOptions;

		std::vector<int> _primNodes;

		//node stuffs
		std::vector<NodeTransform> _nodeTransforms;
		std::vector<int> _nodeParents;
		std::vector<std::vector<int>> _nodeChildren;
		std::vector<unsigned char*> _buffsToDelete;

		//animations
		std::vector<Animation> _animations;
		std::vector<Skin> _skins;
	};

	class VXT_EXPORT glTFDriver : public AssetDriver
	{
		ASSET_DRIVER
	public:
		virtual bool supportsAsset(std::string_view file) const override
		{
			return file.find(".glb") != std::string::npos || file.find(".gltf") != std::string::npos;
		}
		virtual std::string_view driverName() const override
		{
			return "gltf";
		}

		virtual std::shared_ptr<const FileAsset> buildFileAsset(std::string_view file) const override
		{
			auto asset = std::make_shared< glTFModelFile>();
			if (asset->processFile(file))
				return asset;
			return nullptr;
		}
		virtual std::shared_ptr<const DeviceAsset> buildDeviceAsset(std::shared_ptr<const FileAsset> fileAsset, const vkl::Device& device, const vkl::SwapChain& swapChain, vkl::BufferManager& bufferManager) const override
		{
			auto asset = std::make_shared<glTFModel>();
			if (asset->buildAsset(fileAsset, device, swapChain, bufferManager))
				return asset;
			return nullptr;
		}

	};
}
REGISTER_DRIVER(vxt::glTFDriver)