#pragma once
#include <string>
#include "Core/Graphics/Render.h"
#include "Core/Graphics/Window.h"

//App base class
class Kalm
{

public:
	Kalm(const char* name);


	//Render
	void initiateResouces();

	void loop();

	~Kalm();

private:
	Render render;
	Window window;

	std::string name;

};

