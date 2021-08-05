#pragma once
#include <Graphics/Window.h>
#include <Script/Script.h>
#include <Camera/Camera.h>

	
	class CameraController :public Engine::Script {

	public:
		CameraController(std::shared_ptr<Engine::Camera> camera , std::string id);

		void awake();
		void start();
		void update(float timeStep);

	private:
		void updateDirections();
		float YAW = 90;
		float PITCH = -60;
		float ROLL;
		std::shared_ptr<Engine::Camera> camera;

	};

