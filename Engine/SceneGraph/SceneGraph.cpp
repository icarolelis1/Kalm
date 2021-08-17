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
	root->entity->transform.setScale(glm::vec3(1));
}

void SceneGraph::addNode(std::shared_ptr<Node> entity)
{
	root->childs.emplace_back(entity);
	entity->parent = root;
}

void SceneGraph::addNode(std::shared_ptr<Node> p1, std::shared_ptr<Node> p2)
{
	p1->childs.emplace_back(p2);
	p2->parent = p1;
}

void SceneGraph::buildUI(std::shared_ptr<Node> node)
{

	if (ImGui::TreeNode(node->entity->getName())) {

		ImGui::PushID(node->entity->getName());

		
		ImGui::Text("Components");
		ImGui::Spacing();
		node->entity->buildUiRepresentation();
		ImGui::Spacing();

		glm::vec3 position = node->entity->transform.getPosition();
		glm::vec3 scale = node->entity->transform.getScale();

		ImGui::InputFloat3("Position", (float*)glm::value_ptr(position));
		ImGui::InputFloat3("Scale", (float*)glm::value_ptr(scale));

		ImGui::Spacing();

		node->entity->transform.setPosition(position);
		node->entity->transform.setScale(scale);

		ImGui::PopID();

		std::list<std::shared_ptr<Node>>::iterator it = node->childs.begin();

		while (it != node->childs.end()) {
			buildUI(*it);
			it++;
		}

		ImGui::TreePop();
	}

}

void SceneGraph::updateSceneGraph()
{
	updateTransforms(root);
}

void SceneGraph::updateTransforms(std::shared_ptr<Node> node)
{
	if(node->parent)
	node->entity->transform.updateModelMatrix(node->parent->entity->transform);

	else {
		node->entity->transform.updateModelMatrix();
	}
	std::list<std::shared_ptr<Node>>::iterator it = node->childs.begin();
	while (it != node->childs.end()) {

		updateTransforms(*it);
		it++;
	}
}
