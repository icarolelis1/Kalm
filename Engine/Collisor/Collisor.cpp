#include "Collisor.h"

namespace Engine {



	Collisor::Collisor(std::shared_ptr<Engine::Entity> _entity, glm::vec3 posOffset, const char* name) :Component(name), entity(_entity), localPosition(posOffset)
	{
		componentType = COMPONENT_TYPE::COLLISOR;
		collisorMeshPrefix = "internal/";
		meshEntity = std::make_shared<Entity>(name);

	}

	sMesh Collisor::getCollisorMeshRepresentation()
	{
		return mesh;
	}

	std::shared_ptr<Entity> Collisor::getEntity()
	{
		return entity;
	}

	void Collisor::start()
	{
		collisorPosition = entity->transform.getPosition() + localPosition;

		//Position the mesh that visually represent this collisor (helps to debug collision)

		meshEntity->transform.setPosition(collisorPosition);
	}

	void Collisor::setLocalPos(glm::vec3 lp)
	{
		localPosition = lp;
	}

	void Collisor::update(float deltaTime)
	{
		//Position the collisor in reference to it's entity
		collisorPosition = entity->transform.getPosition() + localPosition;

		//Position the mesh that visually represent this collisor (helps to debug collision)
		meshEntity->transform.setPosition(collisorPosition);
	}

	void Collisor::registerCollisionEvent(const std::function<void(Engine::Collisor& col)> f)
	{
		//std::function <void(const Engine::Collisor&)> c;
		//Engine::Event<Engine::Collisor&> e(c);


		publisher.registerEvent(f);

	}


	void Collisor::notifyCollision(Engine::Collisor& other)
	{
		publisher.trigger(other);
	}

	bool Collisor::testCollision(Collisor& c)
	{
		return false;
	}

	COLLISOR_TYPE Collisor::getCollisorType()
	{
		return collisorType;
	}

	glm::vec3 Collisor::getCollisorPosition()
	{
		return collisorPosition;
	}


	//
	//	SphereCollisor::SphereCollisor(Entity* t, float _radius, const char* name) :Collisor(t, glm::vec3(0), name), radius(_radius)
	//	{
	//		std::string mesh = name;
	//		mesh.append(" mesh");
	//
	//		//Create a mesh to visually represent the collisor in the scene. This helps to set the collisor properly
	//		collisorMesh = std::make_shared<Model>(mesh.c_str(), "Aplication\\assets\\utils\\sphereCollisor.obj", INTERNAL);
	//
	//		collisorMesh->transform.setScale(glm::vec3(radius));
	//	}
	//
	//	void SphereCollisor::displayInferface()
	//	{
	//
	//		ImGui::PushID(name);
	//		ImGui::Text(name);
	//		ImGui::InputFloat("Radius", (float*)&radius, .2f);
	//
	//		glm::vec3 position = localPosition;
	//		glm::vec3 scale = collisorMesh->transform.getScale();
	//		glm::vec3 rotation = collisorMesh->transform.getRotation();
	//
	//		ImGui::Spacing();
	//		ImGui::InputFloat3("Local Position", (float*)glm::value_ptr(position));
	//		ImGui::InputFloat3("Local Rotation", (float*)glm::value_ptr(rotation));
	//
	//		ImGui::Spacing();
	//
	//		localPosition = position;
	//		collisorMesh->transform.setScale(glm::vec3(radius));
	//		collisorMesh->transform.setRotation(rotation);
	//
	//		ImGui::PopID();
	//	}
	//
	//	void SphereCollisor::setlocalPos(glm::vec3 pos)
	//	{
	//		localPosition = pos;
	//	}
	//
	//	bool SphereCollisor::testCollision(Collisor& c)
	//	{
	//		switch (c.getCollisorType()) {
	//
	//		case CollisorType::SPHERE_COLLISOR:
	//			SphereCollisor* s = static_cast<SphereCollisor*>(&c);
	//			return this->testCollisionAgainstSphere(*s);
	//			break;
	//
	//		}
	//
	//	}
	//
	//	SphereCollisor::~SphereCollisor()
	//	{
	//		delete event_handle;
	//	}
	//
	//	bool SphereCollisor::testCollisionAgainstSphere(SphereCollisor& c)
	//	{
	//
	//		return (glm::distance(collisorPosition, c.getCollisorPosition()) < (this->radius + c.radius));
	//
	//	}
	//
	//
	//
	//
	//
	//	RayCastCollisor::RayCastCollisor(Entity* t, float length, const char* name) : Collisor(t, glm::vec3(0), name), m_rayLength(length)
	//	{
	//
	//	}
	//
	//	RayCastCollisor::~RayCastCollisor()
	//	{
	//
	//	}
	//
	//	void RayCastCollisor::displayInferface()
	//	{
	//		ImGui::InputFloat("Length", &m_rayLength);
	//		ImGui::InputFloat3("localPos", (float*)glm::value_ptr(localPosition));
	//
	//		collisorMesh->transform.setScale(.01, m_rayLength, .01);
	//	}
	//
	//	void RayCastCollisor::setRayDirection(glm::vec3& dir, float rot)
	//	{
	//		m_rayDirection = dir;
	//		//collisorMesh->transform.setRotation(90,0, rot);
	//	}
	//
	//	bool RayCastCollisor::testCollision(Collisor& other)
	//	{
	//		switch (other.getCollisorType()) {
	//		case CollisorType::SPHERE_COLLISOR:
	//			SphereCollisor* s = static_cast<SphereCollisor*>(&other);
	//			return this->testAgainstSphere(*s);
	//
	//		}
	//		return false;
	//	}
	//
	//	void RayCastCollisor::update(float timeStep)
	//	{
	//		Collisor::update(timeStep);
	//
	//
	//		m_rayStart = collisorPosition;
	//
	//
	//	}
	//
	//	void RayCastCollisor::start()
	//	{
	//
	//		std::string mesh = name;
	//		mesh.append(" mesh");
	//
	//		//Create a mesh to visually represent the collisor in the scene. This helps to set the collisor properly
	//		//collisorMesh = std::make_shared<Model>(mesh.c_str(), "Aplication\\assets\\utils\\rayCollisor.obj", INTERNAL);
	//		//collisorMesh->transform.setScale(glm::vec3(.01,m_rayLength,.01));
	//
	//		m_rayStart = collisorPosition;
	//	}
	//
	//	bool RayCastCollisor::testAgainstSphere(SphereCollisor& other)
	//	{
	//
	//		/*
	//		glm::vec3 center = other.getCollisorPosition();
	//		if (glm::distance(m_rayStart, center) > m_rayLength)return 0;
	//		float radius = other.radius;
	//
	//		float t = glm::dot(center - m_rayStart,m_rayDirection);
	//		glm::vec3 p = m_rayStart + m_rayDirection * t;
	//		float y = glm::distance(center , p);
	//
	//		return (y < radius) ;
	//
	//		*/
	//		glm::vec3 r = m_rayDirection;
	//		glm::vec3 center = other.getCollisorPosition();
	//		float radius = other.radius;
	//
	//
	//		glm::vec3 oc = m_rayStart - center;
	//
	//		if (glm::distance(oc, m_rayStart) > radius)return false;
	//		float a = dot(m_rayDirection, m_rayDirection);
	//		float b = 2.0 * dot(oc, m_rayDirection);
	//		float c = dot(oc, oc) - radius * radius;
	//		float discriminant = b * b - 4 * a * c;
	//		return (discriminant > 0);
	//	};
	//
	//	BoxCollisor::BoxCollisor(Entity* entity, glm::vec3 dimension, const char* name) :Collisor(entity, glm::vec3(0), name), m_dimenson(dimension)
	//	{
	//
	//
	//		std::string mesh = name;
	//		mesh.append(" mesh");
	//
	//		//Create a mesh to visually represent the collisor in the scene. This helps to set the collisor properly
	//		collisorMesh = std::make_shared<Model>(mesh.c_str(), "Aplication\\assets\\utils\\cubeCollisor.obj", INTERNAL);
	//
	//		collisorMesh->transform.setScale(glm::vec3(dimension));
	//	}
	//
	//	void BoxCollisor::start()
	//	{
	//		m_minX = collisorPosition.x - m_dimenson.x; 		m_maxX = collisorPosition.x + m_dimenson.x;
	//		m_minY = collisorPosition.y - m_dimenson.y; 		m_maxY = collisorPosition.y + m_dimenson.y;
	//		m_minZ = collisorPosition.z - m_dimenson.z; 		m_maxZ = collisorPosition.z + m_dimenson.z;
	//
	//	}
	//
	//	void BoxCollisor::update(float timeStep)
	//	{
	//		Collisor::update(timeStep);
	//		start();
	//	}
	//
	//	void BoxCollisor::displayInferface()
	//	{
	//		ImGui::InputFloat3("BoxDimension ", (float*)glm::value_ptr(m_dimenson));
	//
	//		collisorMesh->transform.setScale(m_dimenson);
	//
	//		ImGui::InputFloat3("LocalPos ", (float*)glm::value_ptr(localPosition));
	//
	//
	//	}
	//
	//	bool BoxCollisor::testCollision(TUGEV::Collisor& c)
	//	{
	//		switch (c.getCollisorType()) {
	//
	//		case CollisorType::SPHERE_COLLISOR:
	//			SphereCollisor* s = static_cast<SphereCollisor*>(&c);
	//			return this->testCollisionAgainstSphere(*s);
	//			break;
	//
	//		}
	//
	//	}
	//
	//	bool BoxCollisor::testCollisionAgainstSphere(TUGEV::SphereCollisor& col)
	//	{
	//		glm::vec3 center = col.getCollisorPosition();
	//
	//		float x = glm::max(m_minX, glm::min(center.x, m_maxX));
	//		float y = glm::max(m_minY, glm::min(center.y, m_maxY));
	//		float z = glm::max(m_minZ, glm::min(center.z, m_maxZ));
	//
	//		return (glm::distance(glm::vec3(x, y, z), center) < col.radius);
	//
	//	}
	//
	//}