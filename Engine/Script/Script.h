#pragma once
#include <Components/Component.h>
#include <glm/gtc/type_ptr.hpp>

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

