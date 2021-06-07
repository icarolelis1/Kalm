#include "Component.h"

Engine::Component::Component(const char* _id):id(_id)
{

}

std::string Engine::Component::getId()
{
	return id;
}

Engine::COMPONENT_TYPE Engine::Component::getTypeOfComponent()
{
	return componentType;
}
