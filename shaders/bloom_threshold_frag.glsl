#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

in vec2 texCoord;

uniform sampler2D framebuffer;

void main(void)
{
    vec3 tex_color = texture(framebuffer,texCoord).rbg;
    vec4 BrightColor;
    float brightness = dot(tex_color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.9)
        BrightColor = vec4(tex_color.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    color = BrightColor;

}
