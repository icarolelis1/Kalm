#include <Entity/Entity.h>
#include <MeshComponent/Mesh.h>
#include <Event/Event.h>

namespace Engine {

	enum class COLLISOR_TYPE

	{
		SPHERE_COLLISOR,
		BOX_COLLISOR
	};

	using sEntity = std::shared_ptr<Engine::Entity>;
	using sMesh = std::shared_ptr<Mesh>;

	class Collisor : public Component {

	public:

		void start();
		void update(float deltaTime);
		Collisor(std::shared_ptr<Engine::Entity> _entity, glm::vec3 posOffset, const char* name);
		sMesh getCollisorMeshRepresentation();
		sEntity getEntity();
		void setLocalPos(glm::vec3);
		void registerCollisionEvent(const std::function<void(Engine::Collisor& col)> f);
		void notifyCollision(Engine::Collisor& other);
		bool testCollision(Collisor& c);
		COLLISOR_TYPE getCollisorType();
		glm::vec3 getCollisorPosition();


	private:


		Engine::Publisher<Engine::Collisor&> publisher;
		sEntity meshEntity;
		Transform transform;
		sMesh mesh;
		glm::vec3 collisorPosition;
		std::string collisorMeshPrefix;
		COLLISOR_TYPE collisorType;
		glm::vec3 localPosition;
		sEntity entity;
	};
}