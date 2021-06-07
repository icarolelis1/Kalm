#pragma once
#include <Entity/Entity.h>
class Node {

public:
	Node(std::shared_ptr<Engine::Entity> entity);
	Node();
	void setParent(std::shared_ptr<Engine::Entity> entity);

	std::list< std::shared_ptr<Engine::Entity>> childs;

	std::shared_ptr<Engine::Entity> entity;
	std::weak_ptr<Engine::Entity> parent;


};


class SceneGraph
{

public:
	SceneGraph();

	void addEntity(std::shared_ptr<Engine::Entity> entity);
	Node root;


private:

};

