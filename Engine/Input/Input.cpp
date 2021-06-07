#include "Input.h"

Engine::Mouse& Engine::Mouse::getInstance()
{
	
	static Mouse instance;
	return instance;
}

void Engine::Mouse::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!initialized)
	{
		lastX = xpos;
		lastY = ypos;
		initialized = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	x = xoffset;
	y = yoffset;
	lastX = xpos;
	lastY = ypos;
}

Engine::KeyBoard::KeyBoard()
{
}

bool Engine::KeyBoard::getKeyPressed(int key)
{

	if (glfwGetKey(window, key)) {
		return 1;
	}		
	return 0;
}

void Engine::KeyBoard::setWindowHandle(GLFWwindow* w)
{
	window = w;
}

void Engine::KeyBoard::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{}
