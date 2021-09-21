#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0 ) out vec2 depthSquared;

void main(){


depthSquared = vec2(gl_FragCoord.z,pow(gl_FragCoord.z,2));
	
}