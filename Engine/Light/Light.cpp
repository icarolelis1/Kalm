#include "Light.h"

Engine::Light::Light(const char*id,glm::vec3 _color, glm::vec3 position, float _type):Entity(id)
{
	transform.setPosition(position);
	type = _type;
	color = _color;

}

void Engine::Light::buildUiRepresentation()
{

	ImGui::InputFloat3("Color", (float*)glm::value_ptr(color));
	ImGui::InputFloat("Type", (float*)&type);

}
