#include "Mesh.h"

Engine::Mesh::Mesh(std::shared_ptr<Engine::Entity> _entity, const char* id, const char* _materiaTag, const char* _file, const VK_Objects::Device* _device, VK_Objects::CommandPool* pool) :Component(id), file(_file), device(_device),entity(_entity),materialTag(_materiaTag)
{
	static const int assimpFlags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices;
	scene = importer.ReadFile(file, aiProcess_Triangulate);
	this->componentType = Engine::COMPONENT_TYPE::MESH;
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "error assimp : " << importer.GetErrorString() << std::endl;
		return;
	}
	texture_paths.resize(scene->mNumMaterials);

	loadMeshes();

	VkCommandBuffer cmd = pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)->getCommandBufferHandle();
	createVertexBuffer(cmd);	
	vkFreeCommandBuffers(device->getLogicalDevice(), pool->getPoolHanndle(), 1, &cmd);

	cmd = pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)->getCommandBufferHandle();
	createIndexBuffer(cmd);	
	vkFreeCommandBuffers(device->getLogicalDevice(), pool->getPoolHanndle(), 1, &cmd);


}

void Engine::Mesh::awake(){
}

void Engine::Mesh::start()
{

}

void Engine::Mesh::update(float timeStep)
{
}

void Engine::Mesh::draw(VkCommandBuffer& cmd, PipelineManager& pipeline_manager, MaterialManager& material_manager,uint32_t index)
{

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer->getBufferHandle(), offsets);
	vkCmdBindIndexBuffer(cmd, indexBuffer->getBufferHandle(), 0, VK_INDEX_TYPE_UINT32);

	for (uint32_t i = 0; i < meshes.size(); i++) {
		//bind invidivual material
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_manager["GBUFFER_COMPOSITION"]->getPipelineLayoutHandle()->getHandle(), 1, 1, &material_manager[texture_paths[i].name]->getDescriptorsetAtIndex(i), 0, NULL);

		vkCmdDrawIndexed(cmd, meshes[i].indexCount, 1, static_cast<uint32_t>(meshes[i].indexBase), meshes[i].vertexOffset, 0);

	}

}

void Engine::Mesh::draw(VkCommandBuffer& cmd)
{

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer->getBufferHandle(), offsets);
	vkCmdBindIndexBuffer(cmd, indexBuffer->getBufferHandle(), 0, VK_INDEX_TYPE_UINT32);

	for (uint32_t i = 0; i < meshes.size(); i++) {
		//bind invidivual material
		vkCmdDrawIndexed(cmd, meshes[i].indexCount, 1, static_cast<uint32_t>(meshes[i].indexBase), meshes[i].vertexOffset, 0);

	}

}

bool Engine::Mesh::shouldUpdateOnThisFrame()
{
	return  (updateTransformOnNextFrame||updateTransformOnEveryFrame);
}

void Engine::Mesh::setUpdateOnEveryFrameNextFrame(bool value) {

	updateTransformOnEveryFrame = value;
}

void Engine::Mesh::setMaterialRag(std::string& tag)
{
	materialTag = tag;
}

std::string Engine::Mesh::getMaterialTag()
{
	return materialTag;
}

void Engine::Mesh::setTexParameter(glm::vec2 v, float r)
{
	tex_data.texOffset = v;
	tex_data.roughnessMultiplier = r;
}

Engine::Tex_data Engine::Mesh::getTexData()
{
	return tex_data;
}

std::vector<Engine::FilesPath> Engine::Mesh::getTextureFIles()
{
	return texture_paths;
}

void Engine::Mesh::setUpdateOnNextFrame(bool value) {

	updateTransformOnNextFrame = value;

}

glm::mat4& Engine::Mesh::getModelMatrix() {

	return entity->transform->getModelMatrix();
}

void Engine::Mesh::destroy()
{
	vertexBuffer.reset();
	indexBuffer.reset();
}

Engine::Mesh::~Mesh()
{
	destroy();
}

void Engine::Mesh::loadMeshes()
{
	uint32_t indexBase = 0;
	uint32_t vertexOffset = 0;
	vertices.clear();
	meshes.resize(scene->mNumMeshes);

	

	for (unsigned int i = 0; i < meshes.size(); i++) {
		aiMesh* aMesh = scene->mMeshes[i];
	
			loadMaterial(aMesh);

		meshes[i].indexBase = indexBase;
		meshes[i].vertexOffset = vertexOffset;
		meshes[i].indexCount = aMesh->mNumFaces * 3;
		for (size_t j = 0; j < aMesh->mNumVertices; j++) {

			Vertex vertex;
			vertex.pos = glm::make_vec3(&aMesh->mVertices[j].x);
			vertex.uv = glm::make_vec2(&aMesh->mTextureCoords[0][j].x);
			vertex.uv.y = 1.0 - vertex.uv.y;
			vertex.normal = glm::make_vec3(&aMesh->mNormals[j].x);
		

			vertices.push_back(vertex);
		}
		vertexOffset += aMesh->mNumVertices;


		for (size_t f = 0; f < aMesh->mNumFaces; f++) {

			for (uint32_t j = 0; j < 3; j++)
			{
				indices.push_back(aMesh->mFaces[f].mIndices[j]);
			}
		}

		indexBase += aMesh->mNumFaces * 3;

	}

}

void Engine::Mesh::loadMaterial(aiMesh *aMesh)
{
	aiMaterial* material = scene->mMaterials[aMesh->mMaterialIndex];
	aiString fileBaseColor, fileMetallicRoughness, emisisonMap, normalMap, name;
	material->Get(AI_MATKEY_NAME, name);


	material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &fileBaseColor);
	material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &fileMetallicRoughness);
	material->GetTexture(aiTextureType_NORMALS,0, &normalMap);
	material->GetTexture(aiTextureType_EMISSIVE,0, &emisisonMap);

	if(name.length>0)
		texture_paths[aMesh->mMaterialIndex].name = name.C_Str();
	else {
		texture_paths[aMesh->mMaterialIndex].name = id + "\\ Material";

	}
	if(fileBaseColor.length>0)
	texture_paths[aMesh->mMaterialIndex].diffuseMap = "Assets\\" +id +"\\" + std::string(fileBaseColor.C_Str());
	else {
		texture_paths[aMesh->mMaterialIndex].diffuseMap = "Assets\\common\\white.png";
	}
	if(emisisonMap.length>0)
	texture_paths[aMesh->mMaterialIndex].emissionMap = "Assets\\" +id+ "\\" + std::string(emisisonMap.C_Str());
	else {
		texture_paths[aMesh->mMaterialIndex].emissionMap = "Assets\\common\\black.png";

	}
	if(fileMetallicRoughness.length>0)
	texture_paths[aMesh->mMaterialIndex].metallicMap = "Assets\\"+id +"\\" + std::string(fileMetallicRoughness.C_Str());
	else {
		texture_paths[aMesh->mMaterialIndex].metallicMap = "Assets\\common\\black.png";
	}
	if(fileMetallicRoughness.length>0)
	texture_paths[aMesh->mMaterialIndex].roughnessMap = "Assets\\"+id+"\\" + std::string(fileMetallicRoughness.C_Str());
	else {
		texture_paths[aMesh->mMaterialIndex].roughnessMap = "Assets\\common\\white.png";

	}
	if(normalMap.length>0)
	texture_paths[aMesh->mMaterialIndex].normalMap = "Assets\\"+id+"\\" + std::string(normalMap.C_Str());
	else {
		texture_paths[aMesh->mMaterialIndex].roughnessMap = "Assets\\common\\black.png";
	}
	texture_paths[aMesh->mMaterialIndex].index = aMesh->mMaterialIndex;

	if (fileBaseColor.length== 0)texture_paths[aMesh->mMaterialIndex].diffuseMap = "Assets//common//black.png";
	if (emisisonMap.length ==0)texture_paths[aMesh->mMaterialIndex].emissionMap = "Assets//common//black.png";
	if (fileMetallicRoughness.length ==0)texture_paths[aMesh->mMaterialIndex].metallicMap = "Assets//common//black.png";
	if (fileMetallicRoughness.length ==0)texture_paths[aMesh->mMaterialIndex].roughnessMap = "Assets//common//black.png";
	if (normalMap.length==0)texture_paths[aMesh->mMaterialIndex].normalMap = "Assets//common//black.png";


}

void Engine::Mesh::createVertexBuffer(VkCommandBuffer cmd)
{
	VkDeviceSize size = sizeof(Vertex) * vertices.size();

	//VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
	VK_Objects::Buffer stagingBuffer(device,size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.id = "staging-Vertex";

	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle(), 0, size, 0, &data);
	memcpy(data, vertices.data(), (size_t)size);
	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle());	

	vertexBuffer = std::make_unique<VK_Objects::Buffer>(device, size, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vertexBuffer->id = "vertex-Buffer";
	Vk_Functions::copyBuffer(cmd, stagingBuffer, *vertexBuffer.get(), size,device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));


}

void Engine::Mesh::createIndexBuffer(VkCommandBuffer cmd)
{
	VkDeviceSize size = sizeof(indices[0]) * indices.size();

	//VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
	VK_Objects::Buffer stagingBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.id = "Index-Staging";
	void* data;
	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle(), 0, size, 0, &data);
	memcpy(data, indices.data(), (size_t)size);
	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle());

	indexBuffer = std::make_unique<VK_Objects::Buffer>(device, size, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	Vk_Functions::copyBuffer(cmd, stagingBuffer, *indexBuffer.get(), size, device->getQueueHandle(VK_Objects::QUEUE_TYPE::TRANSFER));
}
