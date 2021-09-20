#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <glm/gtx/quaternion.hpp>
namespace Engine {

	struct Quaternion {
		float w;
		glm::vec3 axis;
	};

	struct EulerDirections {
		glm::vec3 front;
		glm::vec3 right;
		glm::vec3 up;

	};
	class Transform {

	public:

		glm::mat4& getModelMatrix();
		glm::vec3& getPosition();
		glm::vec3& getScale();
		glm::vec3 getRotation();

		void setPosition(glm::vec3 p);
		void setPosition(float x, float y, float z);
		void setScale(glm::vec3 p);
		void setRotation(glm::vec3 r);
		void setRotation(float x, float y, float z);
		void rotateMix(glm::quat q1, glm::quat q2, float dt);

		glm::quat calculateRotationQuaternion(glm::vec3 axis, float angle);

		void increasePos(float x, float y, float z);
		void increasePos(glm::vec3 p);

		void updateModelMatrix(Transform & parent);
		void updateModelMatrix();

		void rotate(glm::vec3 axis, float angle);

	private:
		glm::mat4 rotMatrix = glm::mat4(1.0f);
		glm::mat4 model;
		glm::vec3 scale;
		glm::vec3 position;
		glm::vec3 rotation;

		glm::quat rotQuaternion = glm::vec3(0, 1, 0);
		float angle = 0;
		float rotAngle;

	};

}