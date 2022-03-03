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

Utils::WindowHandler* Window::getWindowHandler()
{
	return windowHandler;
}

void Window::createWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, 0);

	windowHandler = glfwCreateWindow(WIDTH, HEIGHT, title, nullptr, nullptr);
	glfwMakeContextCurrent(windowHandler);
	
	keyboard.setWindowHandle(windowHandler);
	
	mouse.setWindowHandler(windowHandler);

	glfwSetWindowUserPointer(windowHandler, this);

	auto f = [](GLFWwindow* window, double xpos, double ypos) {
		static_cast<Window*>(glfwGetWindowUserPointer(window))->mouse.mouse_callback(window, xpos, ypos);
	};
	auto mouse_btn_callBack = [](GLFWwindow* window, int x,int y,int z) {
		static_cast<Window*>(glfwGetWindowUserPointer(window))->mouse.mouse_button_callback(window, x, y, z);
	};
	glfwSetCursorPosCallback(windowHandler, f);
	glfwSetMouseButtonCallback( windowHandler,mouse_btn_callBack);

}



void Window::destroyWindow()
{
	//glfwDestroyWindow(windowHandler);

}

