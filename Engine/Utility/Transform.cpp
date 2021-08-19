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

void Engine::Transform::setPosition(float x,float y , float z)
{
	position = glm::vec3(x,y,z);
}

void Engine::Transform::setScale(glm::vec3 p)
{
	scale = p;
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
	identity = glm::translate(identity, position);
	identity = glm::rotate(identity, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	identity = glm::rotate(identity, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	identity = glm::rotate(identity, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	identity = glm::scale(identity, scale);
	identity = glm::translate(identity, glm::vec3(0));

	glm::mat4 parentModel = parent.getModelMatrix();
	model = parentModel * identity;

	

}

void Engine::Transform::updateModelMatrix()
{
	glm::mat4 identity(1.0f);
	identity = glm::translate(identity, position);
	identity = glm::rotate(identity, glm::radians(rotation.z), glm::vec3(0, 0, 1));
	identity = glm::rotate(identity, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	identity = glm::rotate(identity, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	identity = glm::scale(identity, scale);
	identity = glm::translate(identity, glm::vec3(0));


	model = identity;
	
}
