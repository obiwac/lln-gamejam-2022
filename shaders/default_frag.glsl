#version 300 es
layout(location=0) out mediump  vec4 color;
//varying mediump vec4 color; // TODO: marche pas à cause de la version :/
void main(void)
{
    color = vec4(1.0,1.0,0.0,1.0);
}