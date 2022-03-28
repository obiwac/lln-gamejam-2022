#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

uniform vec4 tint;

void main(void)
{
    color = vec4(1.0) * tint;
}
