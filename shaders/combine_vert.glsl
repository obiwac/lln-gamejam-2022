#version 300 es
precision mediump float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoord;

out vec2 texCoord;
uniform mat4 shake_mat;
void main(void)
{
    gl_Position = vec4(position, 1.0) * shake_mat ;
    texCoord = textureCoord;
}
