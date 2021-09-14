#version 450

layout(set = 0,binding = 0) uniform sampler2D mainImage;




layout (location = 0 ) in vec2 texCoord ;
layout (location = 0 ) out vec4 outColor ;

void makeSobelkernel(inout vec4[9] n,vec2 coord ){

    float w = 1/1920;
    float h = 1/1055;

    n[0] = texture(mainImage,coord+ vec2( -w, -h));
	n[1] = texture(mainImage,coord+ vec2(0.0, -h));
	n[2] = texture(mainImage,coord+ vec2(  w, -h));
	n[3] = texture(mainImage,coord+ vec2( -w, 0.0));
	n[4] = texture(mainImage,coord);
	n[5] = texture(mainImage,coord+ vec2(  w, 0.0));
	n[6] = texture(mainImage,coord+ vec2( -w, h));
	n[7] = texture(mainImage,coord+ vec2(0.0, h));
	n[8] = texture(mainImage,coord+ vec2(  w, h));

    }

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
	
vec3 textureColor = texture(mainImage,texCoord).rgb;
//vec3 textureColorOffset = texture(mainImage,texCoord).rgb;

//if(length(textureColor - textureColorOffset)>.1){
 //   textureColor = vec3(.0,0,0);
//}

textureColor = uncharted2_filmic(textureColor);
textureColor = pow(textureColor, vec3(1./2.2));

    outColor = vec4(textureColor,1.0); 
    //(texture(mainImage,texCoord).xyz)

}