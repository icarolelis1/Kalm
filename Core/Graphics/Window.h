#pragma once
#include "GLFW/glfw3.h"
#include "Input/Input.h"
#include "VulkanFramework.h"
#include <functional>
struct WindowProperties {

	uint32_t _HEIGHT;
	uint32_t _WIDTH;
	const char* _title;

};


class Window
{
public:
	Window();

	void initiateWindow(WindowProperties& properties);

	Utils::WindowHandler *getWindowHandler();

	void destroyWindow();
	static Engine::Mouse mouse;
	static Engine::KeyBoard keyboard;

private:
	void createWindow();;

	Utils::WindowHandler* windowHandler;

	uint32_t HEIGHT;
	uint32_t WIDTH;
	const char* title;

};

