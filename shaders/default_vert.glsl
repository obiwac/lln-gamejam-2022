#version 300 es

precision mediump float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoord;
layout(location = 2) in vec3 normal_attr;

uniform mat4 mvp_matrix;
out vec2 texCoord;
out vec3 normal;

void main(void)
{
    gl_Position = mvp_matrix * vec4(position, 1.0);
    texCoord = textureCoord;
    normal = normal_attr;
}
