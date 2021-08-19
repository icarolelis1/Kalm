#include "Entity.h"

Engine::Entity::Entity(const char* _name):name(_name)
{
}

void Engine::Entity::start()
{
	Components::iterator it = componentContainer.components.begin();
	
	while (it != componentContainer.components.end()) {
		it->second->start();
		it++;

	}
}

void Engine::Entity::update(float timeStep)
{
	Components::iterator it = componentContainer.components.begin();

	while (it != componentContainer.components.end()) {
		it->second->update(timeStep);
		it++;

	}
}

void Engine::Entity::attachComponent(std::shared_ptr<Engine::Component> component)
{
	componentContainer.addComponenet(component,component->getId());
}

const char* Engine::Entity::getName()
{
	return name;
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

void Engine::Entity::buildUiRepresentation()
{
	

	Components::iterator it = componentContainer.components.begin();


	while (it != componentContainer.components.end()) {

	

		if (ImGui::TreeNode(it->second->getId().c_str())) {
			ImGui::PushID(it->second->getId().c_str());
			it->second->buildUi();
			ImGui::PopID();
			ImGui::TreePop();
		}
		it++;

	}
}

void Engine::Entity::logComponents()
{
	componentContainer.listComponents();
}



