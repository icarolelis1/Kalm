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
		void buildUi();
		void saveState(std::fstream & file);
		void loadState(std::fstream& file);

	private:
		std::shared_ptr<Engine::Transform> transform;
		void resetCamera();
		float velocity = .04f;
		void updateDirections();
		float YAW = 90;
		float PITCH = 0;
		float ROLL;
		std::shared_ptr<Engine::Camera> camera;

	};

