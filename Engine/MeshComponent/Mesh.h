#pragma once
#include "Entity/Entity.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
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

		~Mesh();

	private:
		void loadMeshes();
		void createVertexBuffer(VkCommandBuffer cmd);
		void createIndexBuffer(VkCommandBuffer cmd);

		std::string materialTag;

		VK_Objects::PBuffer vertexBuffer;
		VK_Objects::PBuffer indexBuffer;

		const char* file;
		const VK_Objects::Device* device;
		
		//Assimp scene data
		const aiScene* scene;
		Assimp::Importer importer;
		std::vector<MeshPart> meshes;
		std::vector<Vertex> vertices;
		std::vector<uint32_t>indices;
		
		std::shared_ptr<Engine::Entity> entity;
		

		bool updateTransformOnNextFrame = false;

		bool updateTransformOnEveryFrame = false;
	};
}	