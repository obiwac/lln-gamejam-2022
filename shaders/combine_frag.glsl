#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

in vec2 texCoord;

uniform sampler2D framebuffer;

void main(void)
{
   color = texture(framebuffer,texCoord) * vec4(1.0, 1.0, 1.0, 1.0);
}
