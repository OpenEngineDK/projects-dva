varying vec2 screenUV;

void main(void)
{
    screenUV = gl_Vertex.xy * 0.5 + 0.5;

    gl_Position = gl_Vertex;
}
