#version 450
layout (location = 0 ) in vec2 TexCoords ;

layout (location = 0 ) out vec4 Color;


struct LightUbo {

	vec3 position;
	vec3 color;
	float type;

};

layout(set = 0, binding = 0) uniform UniformBuffLight{

	mat4 invView;
	mat4 invProj;
	LightUbo light[1];

}lightUbo;


layout (set = 1, binding = 0) uniform sampler2D Albedo;
layout (set = 1, binding = 1) uniform sampler2D MetallicRoughness;
layout (set = 1, binding = 2) uniform sampler2D Normal;

//layout(set = 1, binding = 0) uniform sampler2D shadowMapImage;

vec3 getPositionFromDepth(float  inDepth){
   
    float d =inDepth;
    vec4 clipSpace = vec4((TexCoords.xy * 2.0) -1 ,d, 1.0);
    vec4 worldSpace = ( lightUbo.invView * lightUbo.invProj) * clipSpace;
    worldSpace /= worldSpace.w;

	return worldSpace.xyz;

}



void main(){

	Color = texture(Normal,TexCoords)	;
	
	//Color = vec4(1);
}