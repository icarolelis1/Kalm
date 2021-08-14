#pragma once
#include <Entity/Entity.h>
#include "imgui.h"

#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include <glm/gtc/type_ptr.hpp>


class Node {

public:
	Node(std::shared_ptr<Engine::Entity> entity);
	Node();
	std::list< std::shared_ptr<Node>> childs;

	std::shared_ptr<Engine::Entity> entity;
	std::weak_ptr<Node> parent;


};


class SceneGraph
{

public:
	SceneGraph();

	void addNode(std::shared_ptr<Node> entity);
	void addNode(std::shared_ptr<Node> p1, std::shared_ptr<Node> p2);

	void buildUI(std::shared_ptr<Node> node);


	std::shared_ptr<Node> root;


private:

};

