#version 450

layout(set = 0,binding = 0) uniform sampler2D mainImage ;




layout (location = 0 ) in vec2 texCoord ;
layout (location = 0 ) out vec4 outColor ;


#define EDGE_THRESHOLD_MIN .0312
#define EDGE_THRESHOLD_MAX  .125





float rgb2luma(vec3 rgb){
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}





//http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html
vec3 FXAA(){

vec3 colorCenter = texture(mainImage  ,texCoord).rgb;

// Luma at the current fragment
float lumaCenter = rgb2luma(colorCenter);

// Luma at the four direct neighbours of the current fragment.
float lumaDown = rgb2luma(textureOffset(mainImage ,texCoord,ivec2(0,-1)).rgb);
float lumaUp = rgb2luma(textureOffset(mainImage,texCoord,ivec2(0,1)).rgb);
float lumaLeft = rgb2luma(textureOffset(mainImage,texCoord,ivec2(-1,0)).rgb);
float lumaRight = rgb2luma(textureOffset(mainImage,texCoord,ivec2(1,0)).rgb);

// Find the maximum and minimum luma around the current fragment.
float lumaMin = min(lumaCenter,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));
float lumaMax = max(lumaCenter,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));

// Compute the delta.
float lumaRange = lumaMax - lumaMin;


// If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
if(lumaRange < max(EDGE_THRESHOLD_MIN,lumaMax*EDGE_THRESHOLD_MAX)){
    
    return colorCenter;
}

float lumaDownUp = lumaDown + lumaUp;
float lumaLeftRight = lumaLeft + lumaRight;

float lumaDownLeft = rgb2luma(textureOffset(mainImage,texCoord,ivec2(-1,-1)).rgb);
float lumaUpRight = rgb2luma(textureOffset(mainImage, texCoord ,ivec2(1,1)).rgb);
float lumaUpLeft = rgb2luma(textureOffset(mainImage, texCoord ,ivec2(-1,1)).rgb);
float lumaDownRight = rgb2luma(textureOffset(mainImage, texCoord ,ivec2(1,-1)).rgb);

float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
float lumaDownCorners = lumaDownLeft + lumaDownRight;
float lumaRightCorners = lumaDownRight + lumaUpRight;
float lumaUpCorners = lumaUpRight + lumaUpLeft;



float edgeHorizontal =  abs(-2.0 * lumaLeft + lumaLeftCorners)  + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0    + abs(-2.0 * lumaRight + lumaRightCorners);
float edgeVertical =    abs(-2.0 * lumaUp + lumaUpCorners)      + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);

// Is the local edge horizontal or vertical ?
bool isHorizontal = (edgeHorizontal >= edgeVertical);

float luma1 = isHorizontal ? lumaDown : lumaLeft;
float luma2 = isHorizontal ? lumaUp : lumaRight;
// Compute gradients in this direction.
float gradient1 = luma1 - lumaCenter;
float gradient2 = luma2 - lumaCenter;

// Which direction is the steepest ?
bool is1Steepest = abs(gradient1) >= abs(gradient2);

// Gradient in the corresponding direction, normalized.
float gradientScaled = 0.25*max(abs(gradient1),abs(gradient2));



// Choose the step size (one pixel) according to the edge direction.
float stepLength = isHorizontal ? 1440 : 2560;

// Average luma in the correct direction.
float lumaLocalAverage = 0.0;

if(is1Steepest){
    // Switch the direction
    stepLength = - stepLength;
    lumaLocalAverage = 0.5*(luma1 + lumaCenter);
} else {
    lumaLocalAverage = 0.5*(luma2 + lumaCenter);
}

// Shift UV in the correct direction by half a pixel.
vec2 currentUv = texCoord;
if(isHorizontal){
    currentUv.y += stepLength * 0.5;
} else {
    currentUv.x += stepLength * 0.5;
}

vec2 inverseScreenSize = vec2(1.0/2560.,1.0/1440.);
vec2 offset = isHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);
// Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
vec2 uv1 = currentUv - offset;
vec2 uv2 = currentUv + offset;

// Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
float lumaEnd1 = rgb2luma(texture(mainImage,uv1).rgb);
float lumaEnd2 = rgb2luma(texture(mainImage,uv2).rgb);
lumaEnd1 -= lumaLocalAverage;
lumaEnd2 -= lumaLocalAverage;

// If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
bool reached1 = abs(lumaEnd1) >= gradientScaled;
bool reached2 = abs(lumaEnd2) >= gradientScaled;
bool reachedBoth = reached1 && reached2;

// If the side is not reached, we continue to explore in this direction.
if(!reached1){
    uv1 -= offset;
}
if(!reached2){
    uv2 += offset;
}   




if(!reachedBoth){

    for(int i = 2; i < 10; i++){
        // If needed, read luma in 1st direction, compute delta.
        if(!reached1){
            lumaEnd1 = rgb2luma(texture(mainImage, uv1).rgb);
            lumaEnd1 = lumaEnd1 - lumaLocalAverage;
        }
        // If needed, read luma in opposite direction, compute delta.
        if(!reached2){
            lumaEnd2 = rgb2luma(texture(mainImage, uv2).rgb);
            lumaEnd2 = lumaEnd2 - lumaLocalAverage;
        }
        // If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
        reached1 = abs(lumaEnd1) >= gradientScaled;
        reached2 = abs(lumaEnd2) >= gradientScaled;
        reachedBoth = reached1 && reached2;

        // If the side is not reached, we continue to explore in this direction, with a variable quality.
        if(!reached1){
            uv1 -= offset * .5;
        }
        if(!reached2){
            uv2 += offset * .5;
        }

        // If both sides have been reached, stop the exploration.
        if(reachedBoth){ break;}
    }
}



// Compute the distances to each extremity of the edge.
float distance1 = isHorizontal ? (texCoord.x - uv1.x) : (texCoord.y - uv1.y);
float distance2 = isHorizontal ? (uv2.x - texCoord.x) : (uv2.y - texCoord.y);

// In which direction is the extremity of the edge closer ?
bool isDirection1 = distance1 < distance2;
float distanceFinal = min(distance1, distance2);

// Length of the edge.
float edgeThickness = (distance1 + distance2);

// UV offset: read in the direction of the closest side of the edge.
float pixelOffset = - distanceFinal / edgeThickness + 0.5;


// Is the luma at center smaller than the local average ?
bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
// (in the direction of the closer side of the edge.)
bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

// If the luma variation is incorrect, do not offset.
float finalOffset = correctVariation ? pixelOffset : 0.0;

vec2 finalUv = texCoord;
if(isHorizontal){
    finalUv.y += finalOffset * stepLength;
} else {
    finalUv.x += finalOffset * stepLength;
}

vec3 finalColor = texture(mainImage,finalUv).rgb;


return finalColor;

}






























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
	
vec3 textureColor = FXAA();
//vec3 textureColorOffset = texture(mainImage,texCoord).rgb;

//if(length(textureColor - textureColorOffset)>.1){
 //   textureColor = vec3(.0,0,0);
//}

textureColor = uncharted2_filmic(textureColor);
textureColor = pow(textureColor, vec3(1./2.2));

    outColor = vec4(textureColor,1.0); 
    //(texture(mainImage,texCoord).xyz)

}