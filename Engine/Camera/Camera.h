#pragma once

#include "Entity/Entity.h"
#include <array>
namespace Engine {

	class Camera : public Entity{

	public:
		Camera(const char* _id);
		glm::mat4& getViewMatrix();
		glm::mat4& getProjectionMatrix();

		void awake();
		void start();
		void update(float timeStep);
		std::array<float, 6> calculateFrustumInLightSpace(glm::mat4 lightMatrix);
		EulerDirections eulerDirections;
		
	private:
		std::array<glm::vec3, 8> calculateFrustumConers();

		
		float farPlane;
		float nearPlane;
		float fov;
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;

	};

}
