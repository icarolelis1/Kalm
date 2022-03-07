
#include "Cube.h"

Engine::Cube::Cube(const char* id, const VK_Objects::Device* device, VK_Objects::CommandPool* pool):Entity(id)
{
	attachComponent(std::make_shared<Engine::Mesh>(getSharedPointer(),id, "default_material", "Assets\\common\\cube.glb", device, pool));

}
