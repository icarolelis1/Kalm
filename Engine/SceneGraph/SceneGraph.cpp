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
		glm::vec3 rotation = node->entity->transform.getRotation();

		ImGui::InputFloat3("Position", (float*)glm::value_ptr(position));
		ImGui::InputFloat3("Rotation", (float*)glm::value_ptr(rotation));
		ImGui::InputFloat3("Scale", (float*)glm::value_ptr(scale));

		ImGui::Spacing();

		node->entity->transform.setPosition(position);
		node->entity->transform.setScale(scale);
		node->entity->transform.setRotation(rotation);

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

void SceneGraph::saveState(std::shared_ptr<Node> node, std::fstream& saveFile)
{
	glm::vec3 p = node->entity->transform.getPosition();
	saveFile << node->entity->getName() << std::endl;
	saveFile << "Position : " << p.x << " " << p.y << " " << p.z << std::endl;
	std::list<std::shared_ptr<Node>>::iterator it = node->childs.begin();

	Engine::ComponentContainerInstance instance = node->entity->getAllComponents();
	Engine::ComponentContainerInstance::iterator it_components = instance.begin();
	saveFile << "Components\n";	
	while (it_components != instance.end()) {
		it_components->second->saveState(saveFile);

		it_components++;
	}

	while (it != node->childs.end()) {

		saveState(*it,saveFile);
		it++;
	}
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
