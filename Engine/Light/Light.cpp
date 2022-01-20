#include "Light.h"

Engine::Light::Light(const char*id,glm::vec3 _color, glm::vec3 position, float _type):Entity(id)
{
	transform.setPosition(position);
	attachComponent(std::make_shared<Engine::LightComponent>(id,getSharedPointer()));
	std::shared_ptr<Engine::LightComponent> light = std::dynamic_pointer_cast<Engine::LightComponent>(getComponent(Engine::COMPONENT_TYPE::LIGHT));
	light->setPosition(position);
	light->setColor(_color);
	light->setType(_type);
}

void Engine::Light::setLightColor(glm::vec3& color)
{
	std::shared_ptr<Engine::LightComponent> light = std::dynamic_pointer_cast<Engine::LightComponent>(getComponent(Engine::COMPONENT_TYPE::LIGHT));
	light->setColor(color);
}

void Engine::Light::setLightType(float type)
{
	std::shared_ptr<Engine::LightComponent> light = std::dynamic_pointer_cast<Engine::LightComponent>(getComponent(Engine::COMPONENT_TYPE::LIGHT));
	light->setType(type);
}

Engine::LightComponent::LightComponent(const char* id, std::shared_ptr<Engine::Entity> _entity):Component(id),entity(_entity)
{
	this->componentType = COMPONENT_TYPE::LIGHT;
	this->position = entity->transform.getPosition();
	this->factor = 0.002;
}

void Engine::LightComponent::buildUi()
{
	ImGui::InputFloat3("Color", glm::value_ptr(this->color));
	ImGui::InputFloat3("Position", glm::value_ptr(this->position));
	ImGui::InputFloat("Type", (float*)&this->type);
	ImGui::InputFloat("Factor", (float*)&this->factor);

}

void Engine::LightComponent::setPosition(glm::vec3& p)
{
	this->position = p;
}

void Engine::LightComponent::setColor(glm::vec3& p)
{
	this->color = p;
}

void Engine::LightComponent::setType(float& t)
{
	this->type = t;
}

glm::vec3 Engine::LightComponent::getPosition()
{
	return position;
}

glm::vec3 Engine::LightComponent::getColor()
{
	return color;
}

float Engine::LightComponent::getType()
{
	return type;
}
float Engine::LightComponent::getFactor()
{
	return factor;
}

void Engine::LightComponent::awake()
{
}

void Engine::LightComponent::start()
{
}

void Engine::LightComponent::update(float timeStep)
{
}
