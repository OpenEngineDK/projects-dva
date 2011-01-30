const vec4 FOG_COLOR = vec4(0.21, 0.29, 0.27, 1.0);

uniform sampler2D color0;
uniform sampler2DShadow depth;

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

void main(void) {
    
    vec4 color = texture2D(color0, screenUV);
    float depth = shadow2D(depth, vec3(screenUV, 0.0)).x;
    
    gl_FragColor = mix(color, FOG_COLOR, pow(depth, 1000.0));
    gl_FragDepth = depth;
}
