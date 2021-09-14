#include "Collisor/Collisor.h"

namespace Engine {

	class SphereCollsior : public Collisor {

	public:

		SphereCollsior(std::shared_ptr<Engine::Entity> _entity, float radius,glm::vec3 posOffset, const char* name);
		bool testCollision(const Engine::Collisor other) const;
		float getRadius() const;
		
	private:

		bool testeCollisionAgaistSphere(const Engine::SphereCollsior& other) const;

		float radius;
	};

}