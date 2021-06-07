#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0 ) out float depthSquared;

void main(){


depthSquared = pow(gl_FragCoord.z,2);
	
}