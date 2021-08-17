#version 450

layout(set = 0,binding = 0) uniform sampler2D mainImage;




layout (location = 0 ) in vec2 texCoord ;
layout (location = 0 ) out vec4 outColor ;


vec3 uncharted2_tonemap_partial(vec3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}


vec3 uncharted2_filmic (vec3 v)
{
    float exposure_bias =1.6;
    vec3 curr = uncharted2_tonemap_partial(v * exposure_bias);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

void main() {
	
vec3 c  = vec3(uncharted2_filmic(texture(mainImage,texCoord).xyz));
c = pow(c, vec3(1./2.2));


outColor = vec4(c	,1.0);

}