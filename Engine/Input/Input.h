#pragma once
#include "GLFW/glfw3.h"
#include <iostream>
namespace Engine {

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
		static Mouse& getInstance();

		void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		

	

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