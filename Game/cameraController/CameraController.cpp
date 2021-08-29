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

		camera->transform.increasePos(camera->eulerDirections.front * timeStep * velocity);

	}


	if (Window::keyboard.getKeyPressed(GLFW_KEY_W)) {

		camera->transform.increasePos(camera->eulerDirections.front * timeStep * velocity);

	}
	
	if (Window::keyboard.getKeyPressed(GLFW_KEY_S)) {

		camera->transform.increasePos(-camera->eulerDirections.front * timeStep * velocity);
	}
	if (Window::keyboard.getKeyPressed(GLFW_KEY_A)) {
		YAW += velocity * timeStep;
	}
	if (Window::keyboard.getKeyPressed(GLFW_KEY_D)) {
		YAW -= .04f * timeStep;
	}

	if (Window::keyboard.getKeyPressed(GLFW_KEY_R)) {
		PITCH += velocity * timeStep;
	}

	if (Window::keyboard.getKeyPressed(GLFW_KEY_F)) {
		PITCH -= velocity * timeStep;
	}

	if (Window::keyboard.getKeyPressed(GLFW_KEY_SPACE)) {
		camera->transform.increasePos(-camera->eulerDirections.up * timeStep * velocity/5.0f);
	}

	else if (Window::keyboard.getKeyPressed(GLFW_KEY_C)) {
		camera->transform.increasePos(camera->eulerDirections.up * timeStep * velocity / 5.0f);
	}

	if (Window::keyboard.getKeyPressed(GLFW_KEY_1)) {
		resetCamera();
	}

	auto offset = 40;
	Engine::CursorPos cursorPos = Window::mouse.getCursorPos();

	if (Window::mouse.getMouseACtionStatus(GLFW_MOUSE_BUTTON_2, GLFW_PRESS)) {
		if (cursorPos.x >= 1920 - offset) { camera->transform.increasePos(velocity * camera->eulerDirections.right); }
		if (cursorPos.x <= offset) camera->transform.increasePos(-velocity * camera->eulerDirections.right);

		if (cursorPos.y >= 1055 - offset) camera->transform.increasePos(-velocity * glm::normalize(glm::vec3(camera->eulerDirections.front.x, 0, camera->eulerDirections.front.z) * glm::cos(glm::radians(-90 - PITCH))));
		if (cursorPos.y <= (offset)) camera->transform.increasePos(velocity * glm::normalize(glm::vec3(camera->eulerDirections.front.x, 0, camera->eulerDirections.front.z) * glm::cos(glm::radians(-90 - PITCH))));
	}


	updateDirections();
	

}

void CameraController::buildUi()
{
	ImGui::InputFloat("Camera Velocity", &velocity);
	ImGui::InputFloat("Yaw", &YAW);
	ImGui::InputFloat("Pitc", &PITCH);

	ImGui::InputFloat3("Right", (float*)glm::value_ptr(camera->eulerDirections.right));
	ImGui::InputFloat3("Front", (float*)glm::value_ptr(camera->eulerDirections.front));
	ImGui::InputFloat3("Up", (float*)glm::value_ptr(camera->eulerDirections.front));


}

void CameraController::saveState(std::fstream& file)
{
	
	file << "YAW : " << YAW << std::endl;
	file << "PITCH : " << PITCH << std::endl;
	file << "velocity : " << velocity << std::endl;
}

void CameraController::loadState(std::fstream& file)
{
	std::string s;
	while (file>>s) {
		std::cout << s << std::endl;

	}
}

void CameraController::resetCamera()
{
	camera->transform.setPosition(-.44, 1.351, -14.5);
	YAW = 90;
	PITCH = 0;
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
