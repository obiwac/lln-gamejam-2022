#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

in vec3 texCoord;

uniform samplerCube skybox;

void main()
{
    color = texture(skybox, vec3(-texCoord.x, texCoord.y, -texCoord.z));
}