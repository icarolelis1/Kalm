#include "Script.h"

Engine::Script::Script(const char* _id):Component(_id)
{

	componentType = COMPONENT_TYPE::SCRIPT;
}



void Engine::Script::awake()
{
}

void Engine::Script::start()
{
}

void Engine::Script::update(float timeStep)
{
}
