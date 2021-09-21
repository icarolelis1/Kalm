#include "Transform.h"

glm::mat4& Engine::Transform::getModelMatrix()
{
	return model;
}

glm::vec3& Engine::Transform::getPosition()
{
	return position;
}

glm::vec3& Engine::Transform::getScale()
{
	return scale;
}

glm::vec3 Engine::Transform::getRotation()
{
	return  rotation;
}

void Engine::Transform::setPosition(glm::vec3 p)
{
	position = p;

}
void Engine::Transform::setRotation(glm::vec3 r)
{
	rotation = r;
}

void Engine::Transform::setRotation(float x, float y, float z)
{
	rotation = glm::vec3(x, y, z);

}

void Engine::Transform::rotateMix(glm::quat q1, glm::quat q2, float dt)
{
	glm::quat q = glm::mix(q1, q2, dt);

	rotMatrix = glm::toMat4(q);

}

void Engine::Transform::setPosition(float x,float y , float z)
{
	position = glm::vec3(x,y,z);
}

void Engine::Transform::setScale(glm::vec3 p)
{
	scale = p;
}


glm::quat Engine::Transform::calculateRotationQuaternion(glm::vec3 axis, float angle)
{
	glm::quat quat = glm::quat(0.0, position.x, position.y, position.z);

	glm::vec3 p = glm::vec3(glm::sin(angle / 2.)) * glm::normalize(axis);
	glm::quat q2 = glm::quat(glm::cos(angle / 2.0), p.x, p.y, p.z);

	glm::quat t = q2 * quat * glm::inverse(q2);
	return t;
}

void Engine::Transform::increasePos(float x, float y, float z)
{
	position += glm::vec3(x, y, z);
}

void Engine::Transform::increasePos(glm::vec3 p)
{
	position += p;
}

void Engine::Transform::updateModelMatrix(Transform& parent)
{
	glm::mat4 identity(1.0f);
	identity = glm::scale(identity, scale);
	identity = glm::translate(identity, position);
	identity = glm::rotate(identity, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	identity = glm::rotate(identity, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	identity = glm::rotate(identity, glm::radians(rotation.y), glm::vec3(0, 1, 0));

	glm::mat4 parentModel = parent.getModelMatrix();

	model = parent.getModelMatrix()*identity;

}

void Engine::Transform::updateModelMatrix()
{
	glm::mat4 identity(1.0f);

	identity *= rotMatrix;
	identity = glm::translate(identity, position);
	identity = glm::scale(identity, scale);


	model = identity;
	
}

void Engine::Transform::rotate(glm::vec3 axis, float angle)
{
	axis = glm::normalize(axis);
	glm::quat quat = glm::quat(0.0,position.x,position.y,position.z);

	glm::vec3 p = glm::vec3(glm::sin(angle / 2.)) * (axis);
	glm::quat q2 = glm::quat(glm::cos(angle / 2.0), p.x, p.y, p.z);

	glm::quat t =  q2 * quat * glm::inverse(q2);

	rotQuaternion = glm::normalize(t);
	rotMatrix = glm::toMat4(glm::normalize(t));


}