#include "SceneGraph.h"

Node::Node(std::shared_ptr<Engine::Entity> entity)
{
	this->entity = std::move(entity);
		 
}

Node::Node()
{
}

void Node::setParent(std::shared_ptr<Engine::Entity> entity)
{
	this->parent = std::move(entity);
}

SceneGraph::SceneGraph()
{
}

void SceneGraph::addEntity(std::shared_ptr<Engine::Entity> entity)
{
	root.childs.emplace_back(std::move(entity));
}
