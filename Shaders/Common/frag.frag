#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(set = 0, binding = 1) uniform sampler2D tex;

layout (location = 0 ) in vec2 texCoord ;
layout (location = 0 ) out vec4 outColor ;

void main() {
	
	vec3 c = texture(tex,texCoord).rgb;
	outColor =vec4(c ,1);


}