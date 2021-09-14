#include "Component.h"

Engine::Component::Component(const char* _id):id(_id)
{

}

std::string Engine::Component::getId()
{
	return id;
}

void Engine::Component::buildUi()
{
	return;
}

void Engine::Component::saveState(std::fstream& file)
{
}

void Engine::Component::loadState(std::fstream& file)
{
}

Engine::COMPONENT_TYPE Engine::Component::getTypeOfComponent()
{
	return componentType;
}
