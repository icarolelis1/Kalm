#include "SphereCollisor.h"

Engine::SphereCollsior::SphereCollsior(std::shared_ptr<Engine::Entity> _entity, float _radius,glm::vec3 posOffset, const char* name):Collisor(_entity,posOffset,name),radius(_radius)
{
#ifdef _DEBUG
	std::cout << "Sphere created\n";


#endif // !DEBUG

}

bool Engine::SphereCollsior::testCollision(const Engine::Collisor other) const
{
	Engine::COLLISOR_TYPE type = other.getCollisorType();


	switch (type)
	{
	case Engine::COLLISOR_TYPE::SPHERE_COLLISOR: {
		const SphereCollsior* otherCol = dynamic_cast<const Engine::SphereCollsior*>(&other);
		return  testeCollisionAgaistSphere(*otherCol);

		break;
	}
	case Engine::COLLISOR_TYPE::BOX_COLLISOR:
		break;
	default:
		break;
	}

}



float Engine::SphereCollsior::getRadius() const
{
	return radius;
}


bool Engine::SphereCollsior::testeCollisionAgaistSphere(const Engine::SphereCollsior& other) const
{
	return (glm::distance(this->getCollisorPosition(), other.getCollisorPosition()) < (radius + other.getRadius()));

}
