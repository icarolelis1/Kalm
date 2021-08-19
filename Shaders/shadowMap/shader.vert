#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 InUv;



layout(set =1 , binding = 0) uniform UniformBufferObject2{

	
	mat4 model;
	

}ubo_M;

	


layout(set = 0, binding = 0) uniform UniformBuffLight{

	mat4 lightMatrix;
}ubo_L;


void main() {


  vec4 fragPos  = ubo_M.model * vec4(inPosition,1.0) ;
    gl_Position = ubo_L.lightMatrix* fragPos;
  
}