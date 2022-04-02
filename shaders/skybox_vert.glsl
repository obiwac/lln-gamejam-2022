#version 300 es
precision mediump float;

layout(location = 0) in vec3 position;

out vec3 texCoord;
uniform mat4 mvp_matrix;

void main(void)
{
    vec4 pos = mvp_matrix * vec4(position, 1.0);
    gl_Position = pos.xyww;
    texCoord = position;
}
