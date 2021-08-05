#pragma once
#include <iostream>
#include "Components/Component.h"
#include "Utility/Transform.h"
namespace Engine {

	struct ComponentContainer {

	public:
		ComponentContainer() {};
		std::map<std::string, std::shared_ptr<Component> >components;

		std::shared_ptr<Component> getComponent(std::string name) {
			return components[name];
		}

		std::shared_ptr<Component> getComponent(COMPONENT_TYPE type) {

			std::map<std::string, std::shared_ptr<Component> >::iterator it;
			it = components.begin();
			while (it != components.end()) {

				if (it->second->getTypeOfComponent() == type) {
					return it->second;
				}
				it++;
			}

			return NULL;


		};


		void  listComponents() {

			std::map<std::string, std::shared_ptr<Component> >::iterator it;
			it = components.begin();
			while (it != components.end()) {

				std::cout << it->first << std::endl;
				it++;
			}
		};
		void  destroy() {

			std::map<std::string, std::shared_ptr<Component> >::iterator it;
			it = components.begin();
			while (it != components.end()) {
				delete it->second.get();

			}
		};


		void addComponenet(std::shared_ptr<Component>component, std::string name) {
			if (components.count(name) > 0) {
				name.append(std::to_string(components.count(name) + 1));
			}
			components[name] = std::move(component);
		}

		bool checkIfExists(std::string n) {
			return components.find(n) != components.end();
		}
		//render the components data in the interface
		void displayComponents() {
			std::map<std::string, std::shared_ptr<Component>  >::iterator it;

			for (it = components.begin(); it != components.end(); it++) {
			//	it->second->displayInferface();
			}

		}


	};

	class Entity {
	public:
		
		Entity(const char* _name);

		virtual void start() ;

		virtual void update(float timeStep);
		void attachComponent(std::shared_ptr<Engine::Component> component);

		std::shared_ptr<Engine::Component> getComponent(std::string name);
		std::shared_ptr<Engine::Component> getComponent(Engine::COMPONENT_TYPE componentType);
		Transform transform;

		Transform transform;


	protected:
		bool alive = true;
		bool displayOnInspector = true;


		Engine::Components components;
		const char* name;

	private:
		ComponentContainer componentContainer;

	};
	
}