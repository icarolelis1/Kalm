#pragma once
#include <Components/Component.h>

namespace Engine {

	class Script: public Component
	{
	public:
		Script(const char* id);

		void awake();
		void start();
		void update(float timeStep);
	
	private:

	};
}

