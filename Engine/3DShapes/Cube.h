
#include "MeshComponent/Mesh.h"
namespace Engine {

	class Cube : public Entity{
	public:
		Cube(const char* id, const VK_Objects::Device * device,VK_Objects::CommandPool* pool);

	};

}