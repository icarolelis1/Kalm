#include "Camera.h"

Engine::Camera::Camera(const char* _id) :  Engine::Entity(_id)
{
	projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, .1f, 100.f);
}

glm::mat4& Engine::Camera::getViewMatrix()
{
	auto pos =   transform.getPosition();

	viewMatrix = glm::lookAt(pos, pos + eulerDirections.front, eulerDirections.up);

	return viewMatrix;
}

glm::mat4& Engine::Camera::getProjectionMatrix()
{
	return projectionMatrix;
}

void Engine::Camera::awake()
{
}

void Engine::Camera::start()
{
	Entity::start();
}

void Engine::Camera::update(float timeStep)
{
}

void Engine::Camera::setFarplane(float t)
{
	farPlane = t;
}

void Engine::Camera::setNearPlane(float t)
{
	nearPlane = t;
}

float Engine::Camera::getFarPlane()
{
	return farPlane;
}

glm::vec3 Engine::Camera::getCenter()
{
	return center;
}

std::array<float, 6> Engine::Camera::calculateFrustumInLightSpace(glm::mat4 lightMatrix)
{
	std::array<glm::vec3, 8> corners = calculateFrustumConers();
	float maxX, maxY, maxZ;
	float minX, minY, minZ;
	maxX = maxY = maxZ = -10000;
	minX = minY = minZ = 10000;

	for (glm::vec3 point : corners) {
		glm::vec3 frustumPoint = lightMatrix * glm::vec4(point, 1.0);

		if (frustumPoint.x > maxX)maxX = frustumPoint.x;
		if (frustumPoint.x < minX)minX = frustumPoint.x;

		if (frustumPoint.y > maxY)maxY = frustumPoint.y;
		if (frustumPoint.y < minY)minY = frustumPoint.y;

		if (frustumPoint.z > maxZ)maxZ = frustumPoint.z;
		if (frustumPoint.z < minZ)minZ = frustumPoint.z;


	}

	std::array<float, 6> boundries = { minX,maxX,minY,maxY,minZ,maxZ };
	return boundries;
}

std::array<glm::vec3, 8> Engine::Camera::calculateFrustumConers()
{
	fov = 45.0f;
	float tan = glm::tan(glm::radians(fov / 2.));
	float aspectRatio = 1924 / 1055;
	float farPlane = this->transform.getPosition().y + 10.0f;
	float heightNear = 2 * tan * nearPlane;
	float widthNear = heightNear * aspectRatio;
	float heightFar = 2 * tan * farPlane;
	float widthFar = heightFar * aspectRatio;

	glm::vec3 centerNear = transform.getPosition() + eulerDirections.front * nearPlane;
	glm::vec3 centerFar = transform.getPosition() + eulerDirections.front * farPlane;
	center = (centerNear + centerFar) /glm::vec3(2.0f);

	//std::cout << "Center : " << centerFar.x<<" "<<centerFar.y<<" "<<centerFar.z << std::endl;
	//std::cout << "FARPLANE " << farPlane << std::endl;
	std::array<glm::vec3, 8> corners;

	//Near corners
	glm::vec3 up = eulerDirections.up;

	corners[0] = centerNear + up * heightNear / 2.f - eulerDirections.right * widthNear / 2.f;// near top left
	corners[1] = centerNear + up * heightNear / 2.f + eulerDirections.right * widthNear / 2.f;//near top right
	corners[2] = centerNear - up * heightNear / 2.f - eulerDirections.right * widthNear / 2.f;// near bottom left
	corners[3] = centerNear - up * heightNear / 2.f + eulerDirections.right * widthNear / 2.f;// near bottom right
	//Far Corners
	corners[4] = centerFar + up * heightFar / 2.f - eulerDirections.right * widthFar / 2.f;// far top left
	corners[5] = centerFar + up * heightFar / 2.f + eulerDirections.right * widthFar / 2.f;//far top right
	corners[6] = centerFar - up * heightFar / 2.f - eulerDirections.right * widthFar / 2.f;// far bottom left
	corners[7] = centerFar - up * heightFar / 2.f + eulerDirections.right * widthFar / 2.f;// far bottom right

	//
	//std::cout << "-------------------------------------------------------\n";
	//std::cout << corners[0].x << "  " << corners[0].y << " " << corners[0].z << std::endl;
	//std::cout << corners[1].x << "  " << corners[1].y << " " << corners[1].z << std::endl;
	//std::cout << corners[2].x << "  " << corners[2].y << " " << corners[3].z << std::endl;
	//std::cout << corners[3].x << "  " << corners[3].y << " " << corners[3].z << std::endl;
	//std::cout << "-------------------------------------------------------\n";
	//
	return corners;

}
