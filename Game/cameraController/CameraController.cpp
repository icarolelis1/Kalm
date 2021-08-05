#include "CameraController.h"

CameraController::CameraController(std::shared_ptr<Engine::Camera> _camera, std::string id):Script(id.data()),camera(_camera)
{
}

void CameraController::awake()
{
}

void CameraController::start()
{
	camera->transform.setPosition(0., 3., -5.);
	updateDirections();

}

void CameraController::update(float timeStep)
{

	if (Window::keyboard.getKeyPressed(GLFW_KEY_W)) {

		camera->transform.increasePos(camera->eulerDirections.front * timeStep * .04f);

	}
	
	if (Window::keyboard.getKeyPressed(GLFW_KEY_S)) {

		camera->transform.increasePos(-camera->eulerDirections.front * timeStep * .04f);
	}
	if (Window::keyboard.getKeyPressed(GLFW_KEY_A)) {
		YAW += .04f * timeStep;
	}
	if (Window::keyboard.getKeyPressed(GLFW_KEY_D)) {
		YAW -= .04f * timeStep;
	}

	if (Window::keyboard.getKeyPressed(GLFW_KEY_R)) {
		PITCH += .04f * timeStep;
	}

	if (Window::keyboard.getKeyPressed(GLFW_KEY_F)) {
		PITCH -= .04f * timeStep;
	}

	updateDirections();
	

}

void CameraController::updateDirections()
{
	glm::vec3 front;
	front.x = cos(glm::radians(YAW)) * cos(glm::radians(PITCH));
	front.y = sin(glm::radians(PITCH));
	front.z = sin(glm::radians(YAW)) * cos(glm::radians(PITCH));
	camera->eulerDirections.front = glm::normalize(front);

	glm::vec3 WorldUp = glm::vec3(0, -1, 0);
	camera->eulerDirections.right = glm::normalize(glm::cross(camera->eulerDirections.front, WorldUp));
	camera->eulerDirections.up = glm::normalize(glm::cross(camera->eulerDirections.right, camera->eulerDirections.front));
}
