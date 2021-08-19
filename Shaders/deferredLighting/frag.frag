#version 450
layout (location = 0 ) in vec2 TexCoords ;

layout (location = 0 ) out vec4 Color;
layout (location = 1) out vec4 Brightness;

struct LightUbo {

    vec3 lightColor;
	vec3 position;
	
	float type;

};

layout(set = 0, binding = 0) uniform UniformBuffLight{

	mat4 invView;
	mat4 invProj;
    vec3 camera;
	LightUbo lights[1];

}lightUbo;

const float PI = 3.14159265359;

layout(set = 1,  binding = 0) uniform sampler2D shadowMapImage;
layout(set = 1,  binding = 1) uniform samplerCube irradianceMap;
layout(set = 1,  binding = 2) uniform samplerCube  prefilterMap;
layout(set = 1,  binding = 3) uniform sampler2D brdfLUT;
layout (set = 1, binding = 4) uniform sampler2D depthMap;

layout (set = 2, binding = 0) uniform sampler2D Albedo;
layout (set = 2, binding = 1) uniform sampler2D MetallicRoughness;
layout (set = 2, binding = 2) uniform sampler2D Normal;




vec3 getPositionFromDepth(float  inDepth){
   
    float d =inDepth;
    vec4 clipSpace = vec4((TexCoords.xy * 2.0) -1 ,d, 1.0);
    vec4 worldSpace = ( lightUbo.invView * lightUbo.invProj) * clipSpace;
    worldSpace /= worldSpace.w;

	return worldSpace.xyz;

}


// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{ 
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);

}

vec3 prefilteredReflection(vec3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 6.; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilterMap, R, lodf).rgb;
	vec3 b = textureLod(prefilterMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness,int ind,vec3 WorldPos,vec3 albedo)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float  factor = 1.0;




    float distance = distance(WorldPos,lightUbo.lights[ind].position);
   
    
 

	vec3 lightColor = lightUbo.lights[ind].lightColor*1.6f ;

	vec3 color = vec3(0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);		
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		color += (kD * albedo  / PI +   spec) * dotNL * lightColor ; //* VISIBILITY
	}

	return color;
}

    float shadowCalculation(vec4 fragPosLightSpace, float bias)
    {
        vec3 projCoords = fragPosLightSpace.xyz / vec3(fragPosLightSpace.w);

        projCoords = (projCoords * 0.5) + vec3(0.5);

        float closestDepth = (texture(depthMap, projCoords.xy).x * .5) + .5;

        float currentDepth = projCoords.z;

        float shadow = 0.0;
        
        vec2 texelSize = 1/textureSize(depthMap,0);
        
        for(int x = -2;x<=2;x++){
            for(int y = -2;y<=2;y++){
                float pcdepth = texture(depthMap,projCoords.xy + vec2(x,y) * texelSize).r * .5 + .5;
                if(currentDepth +bias> pcdepth)shadow+=.1;
                else{
                    shadow+=1.;
                }
            
            }
        
        }

        return shadow/8.;
       

       // if(currentDepth > closestDepth  )return 0.1;
       //return 1.;
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


vec3 uncharted2_filmic(vec3 v)
{
    float exposure_bias =1.6;
    vec3 curr = uncharted2_tonemap_partial(v * exposure_bias);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}


     mat4 projection = inverse(lightUbo.invProj);





void main(){

    


    vec3 albedo = texture(Albedo,vec2(TexCoords.x,TexCoords.y)).xyz ;
    float w = texture(Albedo,vec2(TexCoords)).w;
    vec3 color  =vec3(0.);

    vec3 N = texture(Normal,vec2(TexCoords.x,TexCoords.y)).xyz ;

    float metallic =   texture(MetallicRoughness,vec2(TexCoords.x,TexCoords.y)).r;    
    float roughness =  texture(MetallicRoughness,vec2(TexCoords.x,TexCoords.y)).g;   

    vec3 WorldPos = getPositionFromDepth(texture(depthMap,TexCoords).r); 
    vec4 fragPosLight = (lightUbo.invView * lightUbo.invProj)* vec4(WorldPos,1.0) ;
    vec3 L = normalize(lightUbo.lights[0].position - WorldPos);

    float shadow = shadowCalculation(fragPosLight,0.04);

    vec3 V = normalize(lightUbo.camera - WorldPos);
    vec3 R = reflect(V, N); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic); 



     // reflectance equation
    vec3 Lo = vec3(0.0);



    for(int i = 0; i < 1; ++i) 

        {

        vec3 L = normalize(lightUbo.lights[i].position - WorldPos);
        Lo += specularContribution(L,V,N,F0,metallic,roughness,i,WorldPos,albedo);

        }  


    vec3 viewPos = lightUbo.camera;

	vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 reflection = prefilteredReflection(R, roughness).rgb; 	
	vec3 irradiance = texture(irradianceMap,N).rgb ;

    vec3 diffuse = ( irradiance )* (albedo);


    vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);
    vec3 specular = reflection * (F * brdf.x + brdf.y);

    // Ambient part
	vec3 kD = 1.0 - F; 
	kD *= 1.0 - metallic;

    vec3 ambient = (kD * diffuse  + specular ) ;
    color = vec3(Lo+ambient)  ;


    Color = mix(vec4(irradiance*10000,1.0),vec4(color,1.0),w);

	
    float level =14.;
    if((0.2126*Color.r) + (0.7152*Color.g) + (0.0722*Color.b) > level)
    {

	    float brightenRatio = 1.0 / max(max(color.r, color.g), color.b);
	    Brightness = vec4(color*brightenRatio,1.);

    } 

    else

    {

    Brightness = vec4(0);

    }

}