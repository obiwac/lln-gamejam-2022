#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

in vec2 texCoord;
in vec3 normal;

uniform vec4 tint;
uniform sampler2D colorMap;

void main(void)
{
    //texture(colorMap,texCoord)
    color = vec4(normal, 1.0);//vec4(0.0,1.0,0.0,1.0);
}
