#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Engine {

	struct EulerDirections {
		glm::vec3 front;
		glm::vec3 right;
		glm::vec3 up;

	};
	class Transform {

	public:

		glm::mat4& getModelMatrix();
		glm::vec3 getPosition();
		glm::vec3 getScale();
		

		void setPosition(glm::vec3 p);
		void setPosition(float x, float y, float z);
		void setScale(glm::vec3 p);
		void setRotation(glm::vec3 r);

		void increasePos(float x, float y, float z);
		void increasePos(glm::vec3 p);

		void updateModelMatrix(Transform * parent);

	private:
		glm::mat4 model;
		glm::vec3 scale;
		glm::vec3 position;
		glm::vec3 rotation;


	};

}