#include "ScriptManager.h"

Engine::ScriptManager::ScriptManager()
{
}

void Engine::ScriptManager::start()
{
	std::unordered_map<std::string, std::shared_ptr<Engine::Script>>::iterator it = scripts.begin();

	while (it != scripts.end()) {
		it->second->start();
	it++;
	}
}

void Engine::ScriptManager::update(float timeStep)
{
	std::unordered_map<std::string, std::shared_ptr<Engine::Script>>::iterator it = scripts.begin();

	while (it != scripts.end()) {
		it->second->update(timeStep);
	it++;
	}
}

void Engine::ScriptManager::insertScript(std::shared_ptr<Engine::Script> script)
{
	std::unordered_map<std::string, std::shared_ptr<Engine::Script>>::iterator it = scripts.begin();

	while (it != scripts.end()) {
		std::cout << "pp\n";
		if (it->first == script->getId()) {
		
			std::cout << " Invalid name for  Script "<<it->first<<" is already taken\n";
			return;
		}
		it++;
	}

	scripts[script->getId()] = script;

	return;



}
