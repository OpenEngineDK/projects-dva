uniform sampler2D color0;
uniform sampler2DShadow depth;

uniform float time;

varying vec2 texCoord;

void main(void) {

    vec4 color = texture2D(color0, texCoord);
    float depth = shadow2D(depth, vec3(texCoord, 0.0)).x;
    
    gl_FragColor = color * 2.0;
    gl_FragDepth = depth;
}
