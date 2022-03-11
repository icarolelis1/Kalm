#include "Camera.h"

Engine::Camera::Camera(const char* _id) :  Engine::Entity(_id)
{
	projectionMatrix = glm::perspective(glm::radians(fov), 1.0f, nearPlane, farPlane);
}

glm::mat4& Engine::Camera::getViewMatrix()
{
	auto pos =   transform->getPosition();

	viewMatrix = glm::lookAt(pos, pos + eulerDirections.front, eulerDirections.up);

	return viewMatrix;
}

glm::mat4& Engine::Camera::getProjectionMatrix()
{
	projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, nearPlane, farPlane);

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

void Engine::Camera::setWidthHeight(int w, int h)
{
	width = w;
	height = h;
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

float Engine::Camera::getNearPlane()
{
	return nearPlane;
}

glm::vec3 Engine::Camera::getCenter()
{
	return center;
}

std::array<float, 6> Engine::Camera::calculateFrustumInLightSpace(glm::mat4 lightMatrix,glm::vec3 shadow_caster)
{
	std::array<glm::vec3, 8> corners = calculateFrustumConers(shadow_caster);
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

std::array<glm::vec3, 8> Engine::Camera::calculateFrustumConers(glm::vec3 shadow_caster)
{

	//Light Space Directions
	glm::vec3 right = glm::normalize(glm::cross(shadow_caster,glm::vec3(0,1,0)));
	glm::vec3 lightUp = glm::vec3(glm::cross(shadow_caster, right));

	float tan = glm::tan(glm::radians(fov / 2.));
	float farPlane = 25.0f;
	float aspectRatio = width / height;
	float heightNear = 2 * tan * nearPlane;
	float widthNear = heightNear * aspectRatio;
	float heightFar = 2 * tan * farPlane;
	float widthFar = heightFar * aspectRatio;

	glm::vec3 centerNear = transform->getPosition() + eulerDirections.front * nearPlane;
	glm::vec3 centerFar = transform->getPosition() + eulerDirections.front * farPlane;
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

Engine::Frustum& Engine::Camera::calculateFrustumPlanes()
{

	const float halfVSide = farPlane * tanf(glm::radians(fov * .5f));
	const float halfHSide = halfVSide * width / height;;
	const glm::vec3 frontMultFar = farPlane * eulerDirections.front;
	glm::vec3 position = transform->getPosition();
	glm::vec3 n = eulerDirections.front;

	frustum.nearPlane = { };
	frustum.nearPlane.distance = glm::dot(position + nearPlane * eulerDirections.front,n);
	frustum.nearPlane.normal = n;

	frustum.farPlane = { };
	n = eulerDirections.front * glm::vec3(-1);

	frustum.farPlane.distance = glm::dot(position + farPlane * eulerDirections.front,n);
	frustum.farPlane.normal = n;

	frustum.rightPlane = { };
	n = glm::normalize(glm::cross(eulerDirections.up, frontMultFar + eulerDirections.right * halfHSide));
	frustum.rightPlane.distance = glm::dot(position,n);
	frustum.rightPlane.normal = n;


	frustum.leftPlane = {};
	n = glm::normalize(glm::cross(frontMultFar - eulerDirections.right * halfHSide, eulerDirections.up));
	frustum.leftPlane.distance = glm::dot(position,n);
	frustum.leftPlane.normal = n;

	frustum.topPlane = { };
	n = glm::normalize(glm::cross(eulerDirections.right, frontMultFar -eulerDirections.up * halfVSide));
	frustum.topPlane.distance = glm::dot(position,n);
	frustum.topPlane.normal = n;

	frustum.bottomPlane = { };
	n = glm::normalize(glm::cross(frontMultFar + eulerDirections.up * halfVSide, eulerDirections.right));
	frustum.bottomPlane.distance = glm::dot(position,n);
	frustum.bottomPlane.normal = n;

	return frustum;
}

Engine::ShadowCamera& Engine::Camera::calculateFrustumForShadowSplit(glm::vec3 light_caster)
{
	float cascadeSplits[4];

	unsigned int SHADOW_MAP_CASCADE_COUNT = 1;
	std::array<float, 1> cascades;
	float cascadeSplitLambda = 0.95f;

	float nearClip = getNearPlane();
	float farClip = getFarPlane();
	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
	}

	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		float splitDist = cascadeSplits[i];

		glm::vec3 frustumCorners[8] = {
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
		};

		// Project frustum corners into world space
		glm::mat4 invCam = glm::inverse(getProjectionMatrix() * getViewMatrix());
		for (uint32_t i = 0; i < 8; i++) {
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
			frustumCorners[i] = invCorner / invCorner.w;
		}

		for (uint32_t i = 0; i < 4; i++) {
			glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
			frustumCorners[i + 4] = frustumCorners[i] + (dist * 1.f);
			frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		}

		// Get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t i = 0; i < 8; i++) {
			frustumCenter += frustumCorners[i];
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t i = 0; i < 8; i++) {
			float distance = glm::length(frustumCorners[i] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;

		glm::vec3 lightDir = normalize(-light_caster);
		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);



		lastSplitDist = cascadeSplits[i];




		ShadowCamera shadowCamera = {};
		shadowCamera.lightView = lightViewMatrix;
		shadowCamera.ortho = lightOrthoMatrix;

		return shadowCamera;
	}
}