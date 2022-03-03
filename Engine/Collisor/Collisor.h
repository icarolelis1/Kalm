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
		void awake();

		Collisor(std::shared_ptr<Engine::Entity> _entity, glm::vec3 posOffset, const char* name);
		virtual sMesh getCollisorMeshRepresentation() const;
		sEntity getEntity();
		void setLocalPos(glm::vec3);
		void registerCollisionEvent(const std::function<void(Engine::Collisor& col)> f);
		void notifyCollision(Engine::Collisor& other);
		bool testCollision(Collisor& c) const;
		COLLISOR_TYPE getCollisorType()const ;
		glm::vec3 getCollisorPosition() const;

	private:


		Engine::Publisher<Engine::Collisor&> publisher;
		sEntity meshEntity;
		std::shared_ptr<Engine::Transform> transform;
		glm::vec3 collisorPosition;
		std::string collisorMeshPrefix;
		COLLISOR_TYPE collisorType;
		glm::vec3 localPosition;
		sEntity entity;

	protected:
		sMesh mesh;

	};
}