#version 450

layout (binding = 0) uniform sampler2D albedo;
layout (binding = 1) uniform sampler2D  normal;
layout (binding = 2) uniform sampler2D   position;
layout (binding = 3) uniform sampler2D   position;
layout (location = 0 ) in vec2 TexCoords noise;

layout (location = 0 ) out vec4 Color;
layout (location = 1) out vec4 Brightness;

struct LightUbo {

    vec3 lightColor;
	vec3 position;
    vec3 typeFactor;


};

layout(set = 0, binding = 0) uniform UniformBuffLight{

	mat4 invView;
	mat4 invProj;
    vec3 camera;
	LightUbo lights[3];
	mat4 lightMatrix;	
int num_lights;

}lightUbo;


void main(){



}