#version 450

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D emissionMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessMap;
layout(set = 1, binding = 3) uniform sampler2D metallicnessMap ;
layout(set = 1, binding = 4) uniform sampler2D normalMap;

layout(location = 0) in vec3 Normals;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 WorldPos;



layout (location = 0 ) out vec4 Albedo;
layout (location = 1 ) out vec2 MetallicRoughness;
layout (location = 2 ) out vec4 Normal;
layout (location = 3 ) out vec4 Emission ;
mat3 getTBN(){

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normals);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return TBN;
}


vec3 getNormalFromMap(vec2 scaleFactor)
{

    vec3 tangentNormal = texture(normalMap,  TexCoords * scaleFactor).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normals);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);

}





void main()
{




Normal = vec4(getNormalFromMap(vec2(1.0)),1.0);
vec3 em =   texture( emissionMap ,TexCoords  ).rgb ;
vec2  metRoughness =   vec2(texture( metallicnessMap, TexCoords ).x, texture( metallicnessMap, TexCoords ).y) ;
Albedo =  vec4(pow(texture( diffuseMap, TexCoords).xyz  ,vec3(2.2)),1.0);

MetallicRoughness= vec2(metRoughness.x*0.0 , metRoughness.y);
Emission  = vec4(em,1.0);

}