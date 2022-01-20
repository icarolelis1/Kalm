#include <iostream>
#include <Entity/Entity.h>
#include <glm/gtc/type_ptr.hpp>


namespace Engine {

	class LightComponent : public Component {
	public:
		LightComponent(const char* id, std::shared_ptr<Engine::Entity>_entity);
		void buildUi();
		void setPosition(glm::vec3& p);
		void setColor(glm::vec3& p);
		void setType(float& t);

		glm::vec3 getPosition();
		glm::vec3 getColor();
		float getType();
		float getFactor();

		void awake() ;
		void start() ;
		void update(float timeStep);
	private:
		std::shared_ptr<Engine::Entity> entity;
		glm::vec3 color;
		glm::vec3 position;
		float type;
		float factor;
	};


	class Light : public Entity {

	public:
		Light(const char* id, glm::vec3 color = glm::vec3(1.0f), glm::vec3 position = glm::vec3(0.0), float type = 0.0);

	private:
	;
	};
}