#version 450 
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 InUv;

layout (location = 0) out vec2 outUV;

layout(set = 0,binding = 0) uniform UniformBufferObject{

	mat4 model;
	mat4 view;
	mat4 projection;

}ubo;

void main() 
{ 
    vec4 fragPos = ubo.model * vec4(inPos,1.0); 
    outUV = InUv;
    gl_Position = ubo.projection * ubo.view * fragPos;
}