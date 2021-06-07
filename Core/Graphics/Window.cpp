#include "Window.h"
Engine::KeyBoard Window::keyboard;
Engine::Mouse Window::mouse;
Window::Window()
{

}

void Window::initiateWindow(WindowProperties& properties)
{
	WIDTH = properties._WIDTH;
	HEIGHT = properties._HEIGHT;
	title = properties._title;

	createWindow();

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){

}
Utils::WindowHandler* Window::getWindowHandler()
{
	return windowHandler;
}

void Window::createWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, 1);

	windowHandler = glfwCreateWindow(WIDTH, HEIGHT, title, nullptr, nullptr);
	glfwMakeContextCurrent(windowHandler);
	
	keyboard.setWindowHandle(windowHandler);
	
	glfwSetWindowUserPointer(windowHandler, this);

	auto f = [](GLFWwindow* window, double xpos, double ypos) {
		static_cast<Window*>(glfwGetWindowUserPointer(window))->mouse.mouse_callback(window, xpos, ypos);
	};
	glfwSetCursorPosCallback(windowHandler, f);


}



void Window::destroyWindow()
{
	glfwDestroyWindow(windowHandler);

}

