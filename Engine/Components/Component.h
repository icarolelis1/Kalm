#pragma once
#include <string>
#include <map>
#include <functional>
namespace Engine {


	

	enum class COMPONENT_TYPE
	{
		SCRIPT,
		COLLISOR,
		MESH
	};

	class Component {

	public:

		Component(const char* id);

		std::string getId();
		virtual void awake() = 0;
		virtual void start()=0;
		virtual void update(float timeStep)=0;
		virtual void buildUi();
		virtual void saveState(std::fstream& file);
		virtual void loadState(std::fstream& file);
		COMPONENT_TYPE getTypeOfComponent();

	protected:
		COMPONENT_TYPE componentType;
		std::string id;

	private:

	};
	using Components = std::map<std::string, std::shared_ptr<Engine::Component>>;

}