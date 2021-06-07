#pragma once
#include <Script/Script.h>
#include <map>
#include <iostream>
namespace Engine {

	class ScriptManager {
	public:
		ScriptManager();

		void start();
		void update(float timeStep);
		
		void insertScript(std::shared_ptr<Engine::Script> script);
		
	private:
		std::unordered_map < std::string, std::shared_ptr<Engine::Script>> scripts;

	};

}