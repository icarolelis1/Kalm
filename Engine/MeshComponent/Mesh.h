#pragma once
#include "Entity/Entity.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include <string>
#include <Components/Component.h>
#include <Graphics/VulkanFramework.h>
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>
#include <mesh.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Graphics/GraphicsUtil/GraphicsUtility.h>
#include "Utility/Transform.h"
#include <Material/Material.h>
#include <assimp/pbrmaterial.h>

using PipelineManager = std::unordered_map<const char*, std::unique_ptr<VK_Objects::Pipeline>>;

using MaterialManager = std::unordered_map<std::string, std::unique_ptr<Engine::Material>>;

namespace Engine {

	struct MeshPart {
		uint32_t vertexOffset;
		uint32_t indexBase;
		uint32_t indexCount;

	};

	struct Material_adjustments {
		//roughness / metallicness / useNormalMap
		glm::vec3 roughMettallicN = glm::vec3(1,1,1);
		glm::vec2 textureMultiplier = glm::vec2(1);

	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	struct Tex_data{
		glm::vec2 texOffset = glm::vec2(1);;
		float roughnessMultiplier = 1;

	};

	class Mesh : public Component{

	public:
		Mesh(std::shared_ptr<Engine::Entity> entity, const char *id,const char* materiaTag, const char* file,const VK_Objects::Device *_device,VK_Objects::CommandPool* pool);
		
		void awake();

		void start();
		
		void update(float timeStep);

		void draw(VkCommandBuffer& cmd, PipelineManager& pipeline_manager, MaterialManager& materialmanager, uint32_t index);

		void draw(VkCommandBuffer& cmd);

		bool shouldUpdateOnThisFrame();

		glm::mat4& getModelMatrix();

		void destroy();

		void setUpdateOnNextFrame(bool value);

		void setUpdateOnEveryFrameNextFrame(bool value);

		void setMaterialRag(std::string& tag);

		std::string getMaterialTag();

		void setTexParameter(glm::vec2 v, float r);

		void setMaterialName(std::string s,int materialIndex=0);

		Tex_data getTexData();

		Material_adjustments& getMaterialSettings();

		std::vector<Engine::FilesPath> getTextureFIles();

		void shouldUseNormalMap(int b);

		~Mesh();

	private:
		void loadMeshes();
		void loadMaterial(aiMesh* aMesh);
		void createVertexBuffer(VkCommandBuffer cmd);
		void createIndexBuffer(VkCommandBuffer cmd);
		bool useNormalMap = true;

		std::string materialTag;

		VK_Objects::PBuffer vertexBuffer;
		VK_Objects::PBuffer indexBuffer;

		const char* file;
		const VK_Objects::Device* device;
		Tex_data tex_data;
		//Assimp scene data
		const aiScene* scene;
		Assimp::Importer importer;
		std::vector<MeshPart> meshes;
		std::vector<Vertex> vertices;
		std::vector<uint32_t>indices;
		std::unique_ptr<Engine::Material> material;
		std::shared_ptr<Engine::Entity> entity;
		
		std::vector<Engine::FilesPath> texture_paths;
		Material_adjustments materialAdjustments;

		bool updateTransformOnNextFrame = false;

		bool updateTransformOnEveryFrame = false;

		int internalMeshesCount = 0;
	};
}	