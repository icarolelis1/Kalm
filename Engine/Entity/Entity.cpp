#include "Entity.h"

Engine::Entity::Entity(const char* _name):name(_name)
{
}

void Engine::Entity::start()
{
	Components::iterator it = components.end();
	
	while (it != components.end()) {
		it->second->start();

	}
}

void Engine::Entity::update(float timeStep)
{
	Components::iterator it = components.end();

	while (it != components.end()) {
		it->second->update(timeStep);

	}
}

void Engine::Entity::attachComponent(std::shared_ptr<Engine::Component> component)
{
	componentContainer.addComponenet(component,component->getId());
}

std::shared_ptr<Engine::Component> Engine::Entity::getComponent(std::string name)
{
	
	return componentContainer.getComponent(name);

}

std::shared_ptr<Engine::Component> Engine::Entity::getComponent(Engine::COMPONENT_TYPE componentType)
{
	
	if (componentContainer.getComponent(componentType)!= NULL) {
		return componentContainer.getComponent(componentType);
	}

	else return NULL;
}



