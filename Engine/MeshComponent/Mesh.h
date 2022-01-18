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

namespace Engine {

	struct MeshPart {
		uint32_t vertexOffset;
		uint32_t indexBase;
		uint32_t indexCount;

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

		void draw(VkCommandBuffer &cmd);

		bool shouldUpdateOnThisFrame();

		glm::mat4& getModelMatrix();

		void destroy();

		void setUpdateOnNextFrame(bool value);

		void setUpdateOnEveryFrameNextFrame(bool value);

		void setMaterialRag(std::string& tag);

		std::string getMaterialTag();

		void setTexParameter(glm::vec2 v, float r);

		Tex_data getTexData();

		Engine::FilesPath& getTextureFIles();

		~Mesh();

	private:
		void loadMeshes();
		void loadMaterial(aiMesh* aMesh);
		void createVertexBuffer(VkCommandBuffer cmd);
		void createIndexBuffer(VkCommandBuffer cmd);

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
		
		Engine::FilesPath texture_paths;

		bool updateTransformOnNextFrame = false;

		bool updateTransformOnEveryFrame = false;
	};
}	