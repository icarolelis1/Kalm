#include "Input.h"

Engine::Mouse& Engine::Mouse::getInstance(GLFWwindow * _window)
{
	
	static Mouse instance;
	instance.window = _window;
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

bool Engine::Mouse::getMouseACtionStatus(int button, int action)
{


	return (glfwGetMouseButton(window, button) == action);
	


}

void Engine::Mouse::setWindowHandler(GLFWwindow* _window)
{
	window = _window;
}

Engine::CursorPos Engine::Mouse::getCursorPos()
{
	CursorPos p = { lastX,lastY };
	return p;

}

GLFWmousebuttonfun Engine::Mouse::mouse_button_callback(GLFWwindow* window, int x, int z, int y)
{
	return GLFWmousebuttonfun();
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
