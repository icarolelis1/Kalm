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
		void setWidthHeight(int w, int h);

		void setFarplane(float t);
		void setNearPlane(float t);
		float getFarPlane();
		float getNearPlane();
		glm::vec3 getCenter();
		std::array<float, 6> calculateFrustumInLightSpace(glm::mat4 lightMatrix,glm::vec3 shadow_caster);
		EulerDirections eulerDirections;
		std::array<glm::vec3, 8> calculateFrustumConers(glm::vec3 shadow_caster);
		
	private:

		glm::vec3 center;
		float farPlane = 60.0;
		float nearPlane = .1;
		float fov;
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		float width;
		float height;

	};

}
