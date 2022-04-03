#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

in vec2 texCoord;

uniform sampler2D framebuffer;
uniform vec4 frame_tint;

void main(void)
{
   color = texture(framebuffer,texCoord) * frame_tint;
}
