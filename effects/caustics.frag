uniform sampler2D color0;
uniform sampler2DShadow depth;

uniform vec3 lightDir;

uniform sampler2D distortionMap;

uniform sampler2D causticsMap;
const float CAUSTICS_SIZE = 0.005;

uniform float time;
uniform mat4 viewProjectionInverse;

varying vec2 screenUV;

vec3 WorldPosFromDepth(in sampler2DShadow depth, in vec2 screenUV, 
                       in mat4 viewProjectionInverse){

    // Get the depth buffer value at this pixel.  
    float zOverW = shadow2D(depth, vec3(screenUV, 0.0)).x;
    // screenPos is the viewport position at this pixel in the range -1 to 1.  
    vec4 screenPos = vec4(screenUV.x * 2.0 - 1.0, 
                          screenUV.y * 2.0 - 1.0,  
                          zOverW * 2.0 - 1.0, 1.0);

    // Transform by the view-projection inverse.  
    vec4 currentPos = viewProjectionInverse * screenPos;

    // World space.
    return currentPos.xyz / currentPos.w;
}

float Intensity(vec3 color){
    float minimum = min(color.x, min(color.y, color.z));
    float maximum = max(color.x, max(color.y, color.z));

    return (minimum + maximum) / 2.0;
}

void main(void) {

    vec4 color = texture2D(color0, screenUV);
    
    vec3 worldPos = WorldPosFromDepth(depth, screenUV, viewProjectionInverse);

    // Distort in screenspace
    worldPos += (texture2D(distortionMap, screenUV + vec2(time / 10000.0, 0.0)).xyz - 0.5);

    // Project world coord onto xz plane ie ocean surface
    vec2 surfaceUV = worldPos.xz + lightDir.xz * worldPos.y;

    vec4 caustics = texture2D(causticsMap, surfaceUV * CAUSTICS_SIZE + vec2(0.0, time / 10000.0));
    caustics += texture2D(causticsMap, surfaceUV * CAUSTICS_SIZE * 1.11 + vec2(time / 15000.0, time / 15000.0));
    caustics *= 0.5;

    gl_FragColor = color + Intensity(color.rgb) * caustics;
    gl_FragDepth = shadow2D(depth, vec3(screenUV, 0.0)).x;
}
