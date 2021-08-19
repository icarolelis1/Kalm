#pragma once
#include "GLFW/glfw3.h"
#include <iostream>
namespace Engine {

	typedef struct {
		double x;
		double y;
	}CursorPos;

	class KeyBoard {
	public:
		KeyBoard();

		bool getKeyPressed(int key);
		void setWindowHandle(GLFWwindow* w);
		void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);



	private:
		GLFWwindow* window;

	};

	class Mouse {
		
	public:
		static Mouse& getInstance(GLFWwindow* _window);

		void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		bool getMouseACtionStatus(int button, int action);
		void setWindowHandler(GLFWwindow* _window);
		
		
		CursorPos getCursorPos();

		GLFWmousebuttonfun mouse_button_callback(GLFWwindow* window, int x, int z, int y);
		GLFWwindow* window;

	private:
		float x;
		float y;
		float lastX;
		float lastY;
		bool initialized = false;
	};

	class InputManager {


	};

}