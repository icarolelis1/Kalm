#include "SceneGraph.h"

Node::Node(std::shared_ptr<Engine::Entity> entity)
{
	this->entity = std::move(entity);
	
}

Node::Node()
{
}



SceneGraph::SceneGraph()
{
	std::shared_ptr<Engine::Entity> e1 = std::make_shared<Engine::Entity>("Root");
	root = std::make_shared<Node>(e1);
}

void SceneGraph::addNode(std::shared_ptr<Node> entity)
{
	root->childs.emplace_back(entity);
}

void SceneGraph::addNode(std::shared_ptr<Node> p1, std::shared_ptr<Node> p2)
{
	p1->childs.emplace_back(p2);
}

void SceneGraph::buildUI(std::shared_ptr<Node> node)
{

	if (ImGui::TreeNode(node->entity->getName())) {

		node->entity->buildUiRepresentation();
		glm::vec3 pos = node->entity->transform.getPosition();
		glm::vec3 scale = node->entity->transform.getScale();

		ImGui::InputFloat3("Position", (float*)glm::value_ptr(pos));
		ImGui::InputFloat3("Scale", (float*)glm::value_ptr(scale));

		std::cout << pos.x << std::endl;

		node->entity->transform.setPosition(pos);
		node->entity->transform.setScale(scale);

		std::list<std::shared_ptr<Node>>::iterator it = node->childs.begin();

		while (it != node->childs.end()) {
			buildUI(*it);
			it++;
		}

		ImGui::TreePop();
	}

}
