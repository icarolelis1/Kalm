#include <iostream>
#include <Entity/Entity.h>
#include <glm/gtc/type_ptr.hpp>


namespace Engine {
	class Light : public Entity {

	public:
		Light(const char* id, glm::vec3 color = glm::vec3(1.0f), glm::vec3 position = glm::vec3(0.0), float type = 0.0);

	private:
		void buildUiRepresentation();
		glm::vec3 color;
		float type;
	};
}